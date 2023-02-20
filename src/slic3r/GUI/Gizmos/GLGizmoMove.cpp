// Include GLGizmoBase.hpp before I18N.hpp as it includes some libigl code, which overrides our localization "L" macro.
#include "GLGizmoMove.hpp"
#include "slic3r/GUI/GLCanvas3D.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#if ENABLE_WORLD_COORDINATE
#include "slic3r/GUI/GUI_ObjectManipulation.hpp"
#endif // ENABLE_WORLD_COORDINATE
#include "slic3r/GUI/Plater.hpp"
#include "libslic3r/Model.hpp"

#include <GL/glew.h>

#include <wx/utils.h> 

namespace Slic3r {
namespace GUI {

const double GLGizmoMove3D::Offset = 10.0;

GLGizmoMove3D::GLGizmoMove3D(GLCanvas3D& parent, const std::string& icon_filename, unsigned int sprite_id)
    : GLGizmoBase(parent, icon_filename, sprite_id)
{}

std::string GLGizmoMove3D::get_tooltip() const
{
#if ENABLE_WORLD_COORDINATE
    if (m_hover_id == 0)
        return "X: " + format(m_displacement.x(), 2);
    else if (m_hover_id == 1)
        return "Y: " + format(m_displacement.y(), 2);
    else if (m_hover_id == 2)
        return "Z: " + format(m_displacement.z(), 2);
    else
        return "";
#else
    const Selection& selection = m_parent.get_selection();
    const bool show_position = selection.is_single_full_instance();
    const Vec3d& position = selection.get_bounding_box().center();

    if (m_hover_id == 0 || m_grabbers[0].dragging)
        return "X: " + format(show_position ? position.x() : m_displacement.x(), 2);
    else if (m_hover_id == 1 || m_grabbers[1].dragging)
        return "Y: " + format(show_position ? position.y() : m_displacement.y(), 2);
    else if (m_hover_id == 2 || m_grabbers[2].dragging)
        return "Z: " + format(show_position ? position.z() : m_displacement.z(), 2);
    else
        return "";
#endif // ENABLE_WORLD_COORDINATE
}

bool GLGizmoMove3D::on_mouse(const wxMouseEvent &mouse_event) {
    return use_grabbers(mouse_event);
}

void GLGizmoMove3D::data_changed() {
    m_grabbers[2].enabled = !m_parent.get_selection().is_wipe_tower();
}

bool GLGizmoMove3D::on_init()
{
    for (int i = 0; i < 3; ++i) {
        m_grabbers.push_back(Grabber());
        m_grabbers.back().extensions = GLGizmoBase::EGrabberExtension::PosZ;
    }

    m_grabbers[0].angles = { 0.0, 0.5 * double(PI), 0.0 };
    m_grabbers[1].angles = { -0.5 * double(PI), 0.0, 0.0 };

    m_shortcut_key = WXK_CONTROL_M;

    return true;
}

std::string GLGizmoMove3D::on_get_name() const
{
    return _u8L("Move");
}

bool GLGizmoMove3D::on_is_activable() const
{
    const Selection& selection = m_parent.get_selection();
    return !selection.is_any_cut_volume() && !selection.is_any_connector() && !selection.is_empty();
}

void GLGizmoMove3D::on_start_dragging()
{
    assert(m_hover_id != -1);

    m_displacement = Vec3d::Zero();
#if ENABLE_WORLD_COORDINATE
    const Selection& selection = m_parent.get_selection();
    const ECoordinatesType coordinates_type = wxGetApp().obj_manipul()->get_coordinates_type();
    if (coordinates_type == ECoordinatesType::World)
        m_starting_drag_position = m_center + m_grabbers[m_hover_id].center;
    else if (coordinates_type == ECoordinatesType::Local && selection.is_single_volume_or_modifier()) {
        const GLVolume& v = *selection.get_first_volume();
        m_starting_drag_position = m_center + v.get_instance_transformation().get_rotation_matrix() * v.get_volume_transformation().get_rotation_matrix() * m_grabbers[m_hover_id].center;
    }
    else {
        const GLVolume& v = *selection.get_first_volume();
        m_starting_drag_position = m_center + v.get_instance_transformation().get_rotation_matrix() * m_grabbers[m_hover_id].center;
    }
    m_starting_box_center = m_center;
    m_starting_box_bottom_center = m_center;
    m_starting_box_bottom_center.z() = m_bounding_box.min.z();
#else
    const BoundingBoxf3& box = m_parent.get_selection().get_bounding_box();
    m_starting_drag_position = m_grabbers[m_hover_id].center;
    m_starting_box_center = box.center();
    m_starting_box_bottom_center = box.center();
    m_starting_box_bottom_center.z() = box.min.z();
#endif // ENABLE_WORLD_COORDINATE
}

void GLGizmoMove3D::on_stop_dragging()
{
    m_parent.do_move(L("Gizmo-Move"));
    m_displacement = Vec3d::Zero();
}

void GLGizmoMove3D::on_dragging(const UpdateData& data)
{
    if (m_hover_id == 0)
        m_displacement.x() = calc_projection(data);
    else if (m_hover_id == 1)
        m_displacement.y() = calc_projection(data);
    else if (m_hover_id == 2)
        m_displacement.z() = calc_projection(data);
        
    Selection &selection = m_parent.get_selection();
#if ENABLE_WORLD_COORDINATE
    TransformationType trafo_type;
    trafo_type.set_relative();
    switch (wxGetApp().obj_manipul()->get_coordinates_type())
    {
    case ECoordinatesType::Instance: { trafo_type.set_instance(); break; }
    case ECoordinatesType::Local: { trafo_type.set_local(); break; }
    default: { break; }
    }
    selection.translate(m_displacement, trafo_type);
#else
    selection.translate(m_displacement);
#endif // ENABLE_WORLD_COORDINATE
}

void GLGizmoMove3D::on_render()
{
    glsafe(::glClear(GL_DEPTH_BUFFER_BIT));
    glsafe(::glEnable(GL_DEPTH_TEST));

#if ENABLE_WORLD_COORDINATE
    const Selection& selection = m_parent.get_selection();
    const auto& [box, box_trafo] = selection.get_bounding_box_in_current_reference_system();
    m_bounding_box = box;
    m_center = box_trafo.translation();
    const Transform3d base_matrix = local_transform(m_parent.get_selection());
    for (int i = 0; i < 3; ++i) {
        m_grabbers[i].matrix = base_matrix;
    }

    const Vec3d zero = Vec3d::Zero();
    const Vec3d half_box_size = 0.5 * m_bounding_box.size();

    // x axis
    m_grabbers[0].center = { half_box_size.x() + Offset, 0.0, 0.0 };
    m_grabbers[0].color = AXES_COLOR[0];

    // y axis
    m_grabbers[1].center = { 0.0, half_box_size.y() + Offset, 0.0 };
    m_grabbers[1].color = AXES_COLOR[1];

    // z axis
    m_grabbers[2].center = { 0.0, 0.0, half_box_size.z() + Offset };
    m_grabbers[2].color = AXES_COLOR[2];
#else
    const Selection& selection = m_parent.get_selection();
    const BoundingBoxf3& box = selection.get_bounding_box();
    const Vec3d& center = box.center();

    // x axis
    m_grabbers[0].center = { box.max.x() + Offset, center.y(), center.z() };
    m_grabbers[0].color = AXES_COLOR[0];

    // y axis
    m_grabbers[1].center = { center.x(), box.max.y() + Offset, center.z() };
    m_grabbers[1].color = AXES_COLOR[1];

    // z axis
    m_grabbers[2].center = { center.x(), center.y(), box.max.z() + Offset };
    m_grabbers[2].color = AXES_COLOR[2];
#endif // ENABLE_WORLD_COORDINATE

#if ENABLE_GL_CORE_PROFILE
    if (!OpenGLManager::get_gl_info().is_core_profile())
#endif // ENABLE_GL_CORE_PROFILE
        glsafe(::glLineWidth((m_hover_id != -1) ? 2.0f : 1.5f));

#if ENABLE_WORLD_COORDINATE
    auto render_grabber_connection = [this, &zero](unsigned int id) {
#else
    auto render_grabber_connection = [this, &center](unsigned int id) {
#endif // ENABLE_WORLD_COORDINATE
        if (m_grabbers[id].enabled) {
#if ENABLE_WORLD_COORDINATE
            if (!m_grabber_connections[id].model.is_initialized() || !m_grabber_connections[id].old_center.isApprox(m_grabbers[id].center)) {
                m_grabber_connections[id].old_center = m_grabbers[id].center;
#else
            if (!m_grabber_connections[id].model.is_initialized() || !m_grabber_connections[id].old_center.isApprox(center)) {
                m_grabber_connections[id].old_center = center;
#endif // ENABLE_WORLD_COORDINATE
                m_grabber_connections[id].model.reset();

                GLModel::Geometry init_data;
                init_data.format = { GLModel::Geometry::EPrimitiveType::Lines, GLModel::Geometry::EVertexLayout::P3 };
                init_data.color = AXES_COLOR[id];
                init_data.vertices.reserve(2);
                init_data.indices.reserve(2);

                // vertices
#if ENABLE_WORLD_COORDINATE
                init_data.add_vertex((Vec3f)zero.cast<float>());
#else
                init_data.add_vertex((Vec3f)center.cast<float>());
#endif // ENABLE_WORLD_COORDINATE
                init_data.add_vertex((Vec3f)m_grabbers[id].center.cast<float>());

                // indices
                init_data.add_line(0, 1);

                m_grabber_connections[id].model.init_from(std::move(init_data));
            }

            m_grabber_connections[id].model.render();
        }
    };

    if (m_hover_id == -1) {
#if ENABLE_GL_CORE_PROFILE
        GLShaderProgram* shader = OpenGLManager::get_gl_info().is_core_profile() ? wxGetApp().get_shader("dashed_thick_lines") : wxGetApp().get_shader("flat");
#else
        GLShaderProgram* shader = wxGetApp().get_shader("flat");
#endif // ENABLE_GL_CORE_PROFILE
        if (shader != nullptr) {
            shader->start_using();
            const Camera& camera = wxGetApp().plater()->get_camera();
#if ENABLE_WORLD_COORDINATE
            shader->set_uniform("view_model_matrix", camera.get_view_matrix() * base_matrix);
#else
            shader->set_uniform("view_model_matrix", camera.get_view_matrix());
#endif // ENABLE_WORLD_COORDINATE
            shader->set_uniform("projection_matrix", camera.get_projection_matrix());
#if ENABLE_GL_CORE_PROFILE
            const std::array<int, 4>& viewport = camera.get_viewport();
            shader->set_uniform("viewport_size", Vec2d(double(viewport[2]), double(viewport[3])));
            shader->set_uniform("width", 0.25f);
            shader->set_uniform("gap_size", 0.0f);
#endif // ENABLE_GL_CORE_PROFILE

            // draw axes
            for (unsigned int i = 0; i < 3; ++i) {
                render_grabber_connection(i);
            }

            shader->stop_using();
        }

        // draw grabbers
#if ENABLE_WORLD_COORDINATE
        render_grabbers(m_bounding_box);
#else
        render_grabbers(box);
#endif // ENABLE_WORLD_COORDINATE
    }
    else {
        // draw axis
#if ENABLE_GL_CORE_PROFILE
        GLShaderProgram* shader = OpenGLManager::get_gl_info().is_core_profile() ? wxGetApp().get_shader("dashed_thick_lines") : wxGetApp().get_shader("flat");
#else
        GLShaderProgram* shader = wxGetApp().get_shader("flat");
#endif // ENABLE_GL_CORE_PROFILE
        if (shader != nullptr) {
            shader->start_using();

            const Camera& camera = wxGetApp().plater()->get_camera();
#if ENABLE_WORLD_COORDINATE
            shader->set_uniform("view_model_matrix", camera.get_view_matrix()* base_matrix);
#else
            shader->set_uniform("view_model_matrix", camera.get_view_matrix());
#endif // ENABLE_WORLD_COORDINATE
            shader->set_uniform("projection_matrix", camera.get_projection_matrix());
#if ENABLE_GL_CORE_PROFILE
            const std::array<int, 4>& viewport = camera.get_viewport();
            shader->set_uniform("viewport_size", Vec2d(double(viewport[2]), double(viewport[3])));
            shader->set_uniform("width", 0.5f);
            shader->set_uniform("gap_size", 0.0f);
#endif // ENABLE_GL_CORE_PROFILE

            render_grabber_connection(m_hover_id);
            shader->stop_using();
        }

        shader = wxGetApp().get_shader("gouraud_light");
        if (shader != nullptr) {
            shader->start_using();
            shader->set_uniform("emission_factor", 0.1f);
            // draw grabber
#if ENABLE_WORLD_COORDINATE
            const Vec3d box_size = m_bounding_box.size();
#else
            const Vec3d box_size = box.size();
#endif // ENABLE_WORLD_COORDINATE
            const float mean_size = (float)((box_size.x() + box_size.y() + box_size.z()) / 3.0);
            m_grabbers[m_hover_id].render(true, mean_size);
            shader->stop_using();
        }
    }
}

void GLGizmoMove3D::on_register_raycasters_for_picking()
{
    // the gizmo grabbers are rendered on top of the scene, so the raytraced picker should take it into account
    m_parent.set_raycaster_gizmos_on_top(true);
}

void GLGizmoMove3D::on_unregister_raycasters_for_picking()
{
    m_parent.set_raycaster_gizmos_on_top(false);
}

double GLGizmoMove3D::calc_projection(const UpdateData& data) const
{
    double projection = 0.0;

    const Vec3d starting_vec = m_starting_drag_position - m_starting_box_center;
    const double len_starting_vec = starting_vec.norm();
    if (len_starting_vec != 0.0) {
        const Vec3d mouse_dir = data.mouse_ray.unit_vector();
        // finds the intersection of the mouse ray with the plane parallel to the camera viewport and passing throught the starting position
        // use ray-plane intersection see i.e. https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection algebric form
        // in our case plane normal and ray direction are the same (orthogonal view)
        // when moving to perspective camera the negative z unit axis of the camera needs to be transformed in world space and used as plane normal
        const Vec3d inters = data.mouse_ray.a + (m_starting_drag_position - data.mouse_ray.a).dot(mouse_dir) * mouse_dir;
        // vector from the starting position to the found intersection
        const Vec3d inters_vec = inters - m_starting_drag_position;

        // finds projection of the vector along the staring direction
        projection = inters_vec.dot(starting_vec.normalized());
    }

    if (wxGetKeyState(WXK_SHIFT))
        projection = m_snap_step * (double)std::round(projection / m_snap_step);

    return projection;
}

#if ENABLE_WORLD_COORDINATE
Transform3d GLGizmoMove3D::local_transform(const Selection& selection) const
{
    Transform3d ret = Geometry::translation_transform(m_center);
    if (!wxGetApp().obj_manipul()->is_world_coordinates()) {
        const GLVolume& v = *selection.get_first_volume();
        Transform3d orient_matrix = v.get_instance_transformation().get_rotation_matrix();
        if (selection.is_single_volume_or_modifier() && wxGetApp().obj_manipul()->is_local_coordinates())
            orient_matrix = orient_matrix * v.get_volume_transformation().get_rotation_matrix();
        ret = ret * orient_matrix;
    }
    return ret;
}
#endif // ENABLE_WORLD_COORDINATE

} // namespace GUI
} // namespace Slic3r