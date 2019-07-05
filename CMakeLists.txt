cmake_minimum_required(VERSION 3.7)
project(autoangel)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Lua53)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

set(COMMON_FILES
        src/elements/config.cpp
        src/elements/data.cpp
        src/pck/package.cpp
        src/util/encoding.cpp
)

# variant for C++14
add_subdirectory(external/variant)
link_libraries(mpark_variant)
include_directories(external/variant/include)

# fmtlib
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
link_libraries(fmt-header-only)

# everything else
include_directories(${CMAKE_SOURCE_DIR} ${ZLIB_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
link_libraries(${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

if (${LUA53_FOUND})
    add_subdirectory(external/sol2)
    add_library(lua_autoangel SHARED
            ${COMMON_FILES}
            src/bindings/lua/api.cpp
    )
    set_target_properties(lua_autoangel PROPERTIES PREFIX "")

    target_compile_definitions(lua_autoangel PRIVATE SOL_CHECK_ARGUMENTS=1)
    target_include_directories(lua_autoangel PRIVATE ${LUA53_INCLUDE_DIR})
    target_link_libraries(lua_autoangel ${LUA53_LIBRARIES} sol2)

    add_custom_command(TARGET lua_autoangel POST_BUILD COMMAND cp $<TARGET_FILE:lua_autoangel> ${CMAKE_SOURCE_DIR}/examples/lua/autoangel.so)
endif()

add_subdirectory(external/pybind11)
pybind11_add_module(py_autoangel
        ${COMMON_FILES}
        src/bindings/py/api.cpp
)
add_custom_command(TARGET py_autoangel POST_BUILD COMMAND cp $<TARGET_FILE:py_autoangel> ${CMAKE_SOURCE_DIR}/examples/py/autoangel.so)
