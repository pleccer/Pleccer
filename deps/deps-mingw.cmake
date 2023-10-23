#/|/ Copyright (c) Prusa Research 2019 - 2021 Tomáš Mészáros @tamasmeszaros, Vojtěch Bubník @bubnikv
#/|/
#/|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
#/|/
set(DEP_CMAKE_OPTS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
set(DEP_BOOST_TOOLSET "gcc")
set(DEP_BITS 64)

find_package(Git REQUIRED)

# TODO make sure to build tbb with -flifetime-dse=1
include("deps-unix-common.cmake")
