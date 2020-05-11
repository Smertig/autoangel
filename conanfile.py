from conans import ConanFile, CMake

class AutoAngelConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [
        "zlib/1.2.11",
        "boost/1.73.0",
        "mpark-variant/1.4.0",
        "fmt/5.3.0",
        "sol2/2.20.6",
        "pybind11/2.4.3",
        "libiconv/1.16"
    ]

    generators = "cmake"
