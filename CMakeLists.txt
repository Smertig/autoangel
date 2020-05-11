cmake_minimum_required(VERSION 3.1)
project(autoangel)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_MODULE_PATH}/conan.cmake)

conan_check()
conan_add_remote(NAME conan-center URL https://conan.bintray.com)

conan_cmake_run(
    CONANFILE conanfile.py
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
)

# everything
include_directories(${CMAKE_SOURCE_DIR})
link_libraries(${CONAN_TARGETS})

# autoangel core
add_library(autoangel_core STATIC
        src/elements/config.cpp
        src/elements/data.cpp
        src/pck/package.cpp
        src/util/encoding.cpp
)

if (WIN32)
	set(PY_MODULE_EXT "pyd")
	set(LUA_MODULE_EXT "dll")
else()
	set(PY_MODULE_EXT "so")
	set(LUA_MODULE_EXT "so")
endif()

### lua binding
add_library(lua_autoangel SHARED
        src/bindings/lua/api.cpp
)
set_target_properties(lua_autoangel PROPERTIES PREFIX "")

target_compile_definitions(lua_autoangel PRIVATE SOL_CHECK_ARGUMENTS=1)
target_link_libraries(lua_autoangel PRIVATE autoangel_core)
add_custom_command(TARGET lua_autoangel POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lua_autoangel> ${CMAKE_SOURCE_DIR}/examples/lua/autoangel.${LUA_MODULE_EXT})

### python binding
pybind11_add_module(py_autoangel
        src/bindings/py/api.cpp
)
target_link_libraries(py_autoangel PRIVATE autoangel_core)
add_custom_command(TARGET py_autoangel POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:py_autoangel> ${CMAKE_SOURCE_DIR}/examples/py/autoangel.${PY_MODULE_EXT})

### examples

add_executable(pck_dump examples/cpp/pck_dump.cpp)
target_link_libraries(pck_dump PRIVATE autoangel_core dl)
