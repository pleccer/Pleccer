///|/ Copyright (c) Prusa Research 2019 - 2023 Lukáš Matěna @lukasmatena, Enrico Turri @enricoturri1966, Oleksandra Iushchenko @YuSanka, Filip Sykala @Jony01
///|/
///|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
///|/
#ifndef slic3r_GLGizmoMove_hpp_
#define slic3r_GLGizmoMove_hpp_

#include "GLGizmoBase.hpp"


namespace Slic3r {
namespace GUI {

class Selection;

class GLGizmoMove3D : public GLGizmoBase
{
    static const double Offset;

    Vec3d m_displacement{ Vec3d::Zero() };
    Vec3d m_center{ Vec3d::Zero() };
    BoundingBoxf3 m_bounding_box;
    double m_snap_step{ 1.0 };
    Vec3d m_starting_drag_position{ Vec3d::Zero() };
    Vec3d m_starting_box_center{ Vec3d::Zero() };
    Vec3d m_starting_box_bottom_center{ Vec3d::Zero() };

    struct GrabberConnection
    {
        GLModel model;
        Vec3d old_center{ Vec3d::Zero() };
    };
    std::array<GrabberConnection, 3> m_grabber_connections;

public:
    GLGizmoMove3D(GLCanvas3D& parent, const std::string& icon_filename, unsigned int sprite_id);
    virtual ~GLGizmoMove3D() = default;

    double get_snap_step(double step) const { return m_snap_step; }
    void set_snap_step(double step) { m_snap_step = step; }

    std::string get_tooltip() const override;

    /// <summary>
    /// Postpone to Grabber for move
    /// </summary>
    /// <param name="mouse_event">Keep information about mouse click</param>
    /// <returns>Return True when use the information otherwise False.</returns>
    bool on_mouse(const wxMouseEvent &mouse_event) override;

    /// <summary>
    /// Detect reduction of move for wipetover on selection change
    /// </summary>
    void data_changed(bool is_serializing) override;
protected:
    bool on_init() override;
    std::string on_get_name() const override;
    bool on_is_activable() const override;
    void on_start_dragging() override;
    void on_stop_dragging() override;
    void on_dragging(const UpdateData& data) override;
    void on_render() override;
    virtual void on_register_raycasters_for_picking() override;
    virtual void on_unregister_raycasters_for_picking() override;

private:
    double calc_projection(const UpdateData& data) const;
    Transform3d local_transform(const Selection& selection) const;
};


} // namespace GUI
} // namespace Slic3r

#endif // slic3r_GLGizmoMove_hpp_