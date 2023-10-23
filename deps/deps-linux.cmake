#/|/ Copyright (c) Prusa Research 2018 - 2021 Tomáš Mészáros @tamasmeszaros, Vojtěch Bubník @bubnikv, Vojtěch Král @vojtechkral, David Kocík @kocikdav
#/|/
#/|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
#/|/

set(DEP_CMAKE_OPTS "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")

include("deps-unix-common.cmake")

# Some Linuxes may have very old libpng, so it's best to bundle it instead of relying on the system version.
# find_package(PNG QUIET)
# if (NOT PNG_FOUND)
#     message(WARNING "No PNG dev package found in system, building static library. You should install the system package.")
# endif ()