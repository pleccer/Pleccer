///|/ Copyright (c) Prusa Research 2019 - 2021 Tomáš Mészáros @tamasmeszaros, Vojtěch Bubník @bubnikv
///|/
///|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
///|/
#ifndef slic3r_Tesselate_hpp_
#define slic3r_Tesselate_hpp_

#include <vector>
#include <admesh/stl.h>

#include "ExPolygon.hpp"

namespace Slic3r {

const bool constexpr NORMALS_UP = false;
const bool constexpr NORMALS_DOWN = true;

extern std::vector<Vec3d> triangulate_expolygon_3d (const ExPolygon  &poly,  coordf_t z = 0, bool flip = NORMALS_UP);
extern std::vector<Vec3d> triangulate_expolygons_3d(const ExPolygons &polys, coordf_t z = 0, bool flip = NORMALS_UP);
extern std::vector<Vec2d> triangulate_expolygon_2d (const ExPolygon  &poly,  bool flip = NORMALS_UP);
extern std::vector<Vec2d> triangulate_expolygons_2d(const ExPolygons &polys, bool flip = NORMALS_UP);
extern std::vector<Vec2f> triangulate_expolygon_2f (const ExPolygon  &poly,  bool flip = NORMALS_UP);
extern std::vector<Vec2f> triangulate_expolygons_2f(const ExPolygons &polys, bool flip = NORMALS_UP);

indexed_triangle_set wall_strip(const Polygon &poly,
                                double         lower_z_mm,
                                double         upper_z_mm);

} // namespace Slic3r

#endif /* slic3r_Tesselate_hpp_ */
