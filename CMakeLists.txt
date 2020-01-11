cmake_minimum_required(VERSION 3.1)
project(autoangel)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_MODULE_PATH}/conan.cmake)

conan_check()
conan_add_remote(NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_add_remote(NAME zimmerk URL https://api.bintray.com/conan/zimmerk/conan)

conan_cmake_run(
    REQUIRES
        zlib/1.2.11@conan/stable
        boost/1.68.0@conan/stable
        variant/1.3.0@bincrafters/stable
        fmt/5.3.0@bincrafters/stable
        sol2/2.20.4@zimmerk/stable
        pybind11/2.3.0@conan/stable		
        libiconv/1.15@bincrafters/stable
    OPTIONS
        sol2:single_header=True
    BASIC_SETUP
    BUILD missing
)

find_package(pybind11)

# everything
include_directories(${CMAKE_SOURCE_DIR})
link_libraries(${CONAN_LIBS})

# autoangel core
add_library(autoangel_core STATIC
        src/elements/config.cpp
        src/elements/data.cpp
        src/pck/package.cpp
        src/util/encoding.cpp
)

### lua binding
add_library(lua_autoangel SHARED
        src/bindings/lua/api.cpp
)
set_target_properties(lua_autoangel PROPERTIES PREFIX "")

target_compile_definitions(lua_autoangel PRIVATE SOL_CHECK_ARGUMENTS=1)
target_link_libraries(lua_autoangel PRIVATE autoangel_core)
add_custom_command(TARGET lua_autoangel POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lua_autoangel> ${CMAKE_SOURCE_DIR}/examples/lua/autoangel.so)

### python binding
if (WIN32)
	set(PY_MODULE_EXT "pyd")
else()
	set(PY_MODULE_EXT "so")
endif()

pybind11_add_module(py_autoangel
        src/bindings/py/api.cpp
)
target_link_libraries(py_autoangel PRIVATE autoangel_core)	
add_custom_command(TARGET py_autoangel POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:py_autoangel> ${CMAKE_SOURCE_DIR}/examples/py/autoangel.${PY_MODULE_EXT})

### examples

add_executable(pck_dump examples/cpp/pck_dump.cpp)
target_link_libraries(pck_dump PRIVATE autoangel_core dl)
