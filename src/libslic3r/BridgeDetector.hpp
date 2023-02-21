#ifndef slic3r_BridgeDetector_hpp_
#define slic3r_BridgeDetector_hpp_

#include "PrintConfig.hpp"
#include "ClipperUtils.hpp"
#include "Line.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Polyline.hpp"
#include "PrincipalComponents2D.hpp"
#include "libslic3r.h"
#include "ExPolygon.hpp"
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace Slic3r {

// The bridge detector optimizes a direction of bridges over a region or a set of regions.
// A bridge direction is considered optimal, if the length of the lines strang over the region is maximal.
// This is optimal if the bridge is supported in a single direction only, but
// it may not likely be optimal, if the bridge region is supported from all sides. Then an optimal 
// solution would find a direction with shortest bridges.
// The bridge orientation is measured CCW from the X axis.
class BridgeDetector {
public:
    // The non-grown holes.
    const ExPolygons            &expolygons;
    // In case the caller gaves us the input polygons by a value, make a copy.
    ExPolygons                   expolygons_owned;
    // Lower slices, all regions.
    const ExPolygons   			&lower_slices;
    bool is_bridge {false};
    // Scaled extrusion width of the infill.
    coord_t                      spacing;
    // Angle resolution for the brute force search of the best bridging angle.
    double                       resolution;
    // The final optimal angle.
    double                       angle;
    
    BridgeDetector(ExPolygon _expolygon, const ExPolygons &_lower_slices, coord_t _extrusion_width);
    BridgeDetector(const ExPolygons &_expolygons, const ExPolygons &_lower_slices, coord_t _extrusion_width);
    // If bridge_direction_override != 0, then the angle is used instead of auto-detect.
    bool detect_angle(double bridge_direction_override = 0, const PrintRegionConfig *params = nullptr);
    Polygons coverage(double angle = -1, bool precise = true) const;
    void unsupported_edges(double angle, Polylines* unsupported) const;
    Polylines unsupported_edges(double angle = -1) const;
    
private:
    // Suppress warning "assignment operator could not be generated"
    BridgeDetector& operator=(const BridgeDetector &);

    void initialize();

    struct BridgeDirection {
        BridgeDirection(double a = -1., float along_perimeter = 0) : angle(a), coverage(0.), along_perimeter_length(along_perimeter){}

        double angle;
        double coverage;
        float along_perimeter_length;
        Polyline _pedestal;
	bool has_overhang_holes = false;
        coordf_t total_length_anchored = 0;
        coordf_t median_length_anchor = 0;
        coordf_t max_length_anchored = 0;
        uint32_t nb_lines_anchored = 0;
        coordf_t total_length_free = 0;
        coordf_t max_length_free = 0;
        uint32_t nb_lines_free = 0;
    };
public:
    // Get possible briging direction candidates.
    std::vector<BridgeDirection> bridge_direction_candidates(bool only_from_polygon = false) const;

    // Open lines representing the supporting edges.
    Polylines _edges;

    Polyline _pedestal;
    bool has_overhang_holes {false};
    // Closed polygons representing the supporting areas.
    ExPolygons _anchor_regions;
};

//return ideal bridge direction and unsupported bridge endpoints distance.
inline std::tuple<Vec2d, double> detect_bridging_direction(const Polygons &to_cover, const Polygons &anchors_area)
{
    Polygons  overhang_area      = diff(to_cover, anchors_area);
    Polylines floating_polylines = diff_pl(to_polylines(overhang_area), expand(anchors_area, float(SCALED_EPSILON)));

    if (floating_polylines.empty()) {
        // consider this area anchored from all sides, pick bridging direction that will likely yield shortest bridges
        auto [pc1, pc2] = compute_principal_components(overhang_area);
        if (pc2 == Vec2f::Zero()) { // overhang may be smaller than resolution. In this case, any direction is ok
            return {Vec2d{1.0,0.0}, 0.0};
        } else {
            return {pc2.normalized().cast<double>(), 0.0};
        }
    }

    // Overhang is not fully surrounded by anchors, in that case, find such direction that will minimize the number of bridge ends/180turns in the air
    Lines     floating_edges     = to_lines(floating_polylines);
    std::unordered_map<double, Vec2d> directions{};
    for (const Line &l : floating_edges) {
        Vec2d normal = l.normal().cast<double>().normalized();
        double quantized_angle = std::ceil(std::atan2(normal.y(),normal.x()) * 1000.0);
        directions.emplace(quantized_angle, normal);
    }
    std::vector<std::pair<Vec2d, double>> direction_costs{};
    // it is acutally cost of a perpendicular bridge direction - we find the minimal cost and then return the perpendicular dir
    for (const auto& d : directions) {
        direction_costs.emplace_back(d.second, 0.0);
    }

    for (const Line &l : floating_edges) {
        Vec2d line = (l.b - l.a).cast<double>();
        for (auto &dir_cost : direction_costs) {
            // the dot product already contains the length of the line. dir_cost.first is normalized.
            dir_cost.second += std::abs(line.dot(dir_cost.first));
        }
    }

    Vec2d  result_dir = Vec2d::Ones();
    double min_cost   = std::numeric_limits<double>::max();
    for (const auto &cost : direction_costs) {
        if (cost.second < min_cost) {
            // now flip the orientation back and return the direction of the bridge extrusions
            result_dir = Vec2d{cost.first.y(), -cost.first.x()};
            min_cost   = cost.second;
        }
    }

    return {result_dir, min_cost};
};
}

#endif
