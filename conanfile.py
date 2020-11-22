from conans import ConanFile, CMake

class AutoAngelConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    requires = [
        "zlib/1.2.11",
        "fmt/7.1.2",
        "sol2/3.2.2",
        "pybind11/2.6.1",
        "libiconv/1.16"
    ]

    generators = "cmake"
