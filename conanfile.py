from conan import ConanFile
from conan.tools.cmake import cmake_layout

class NetworkingRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    requires = "asio/1.32.0", "gtest/1.15.0"

    def layout(self):
        cmake_layout(self)
