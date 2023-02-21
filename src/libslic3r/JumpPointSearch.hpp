#ifndef SRC_LIBSLIC3R_JUMPPOINTSEARCH_HPP_
#define SRC_LIBSLIC3R_JUMPPOINTSEARCH_HPP_

#include "BoundingBox.hpp"
#include "libslic3r/Layer.hpp"
#include "libslic3r/Point.hpp"
#include "libslic3r/Polyline.hpp"
#include "libslic3r/libslic3r.h"
#include <unordered_map>
#include <unordered_set>

namespace Slic3r {

class JPSPathFinder
{
    using Pixel = Point;
    std::unordered_set<Pixel, PointHash> inpassable;
    coordf_t print_z;
    Pixel obstacle_min;
    Pixel obstacle_max;

    const coord_t resolution = scaled(1.5);
    Pixel         pixelize(const Point &p) { return p / resolution; }
    Point         unpixelize(const Pixel &p) { return p * resolution; }

public:
    JPSPathFinder() { clear(); };
    void clear();
    void add_obstacles(const Lines &obstacles);
    void add_obstacles(const Layer* layer, const Point& global_origin);
    Polyline find_path(const Point &start, const Point &end);
};

} // namespace Slic3r

#endif /* SRC_LIBSLIC3R_JUMPPOINTSEARCH_HPP_ */
