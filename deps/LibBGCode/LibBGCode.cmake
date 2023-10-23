set(LibBGCode_SOURCE_DIR "" CACHE PATH "Optionally specify local LibBGCode source directory")

set(_source_dir_line
    URL https://github.com/prusa3d/libbgcode/archive/50bedae2ae0c7fc83dd350a8be99ddc8f1749005.zip
	URL_HASH SHA256=3958c93a325d6d7ed1c97aabb37cc09a08f8e981e3a7917312d568071e462162
)

if (LibBGCode_SOURCE_DIR)
    set(_source_dir_line "SOURCE_DIR;${LibBGCode_SOURCE_DIR};BUILD_ALWAYS;ON")
endif ()

prusaslicer_add_cmake_project(LibBGCode_deps
    ${_source_dir_line}
    SOURCE_SUBDIR deps
    DEPENDS dep_Boost ${ZLIB_PKG}
    CMAKE_ARGS
        -DDEP_DOWNLOAD_DIR:PATH=${DEP_DOWNLOAD_DIR}
        -DDEP_CMAKE_OPTS:STRING=-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DLibBGCode_Deps_SELECT_ALL:BOOL=OFF
        -DLibBGCode_Deps_SELECT_heatshrink:BOOL=ON
        -DDESTDIR=${DESTDIR}
)

prusaslicer_add_cmake_project(LibBGCode
    ${_source_dir_line}
    DEPENDS dep_LibBGCode_deps
    CMAKE_ARGS
        -DLibBGCode_BUILD_TESTS:BOOL=OFF
        -DLibBGCode_BUILD_CMD_TOOL:BOOL=OFF
)

if (MSVC)
    add_debug_dep(dep_LibBGCode)
endif ()