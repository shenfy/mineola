from conans import ConanFile, CMake, tools
import os

class MineolaConan(ConanFile):
    name = "mineola_pc"
    version = "1.1.0"
    license = "MIT"
    author = "Fangyang Shen dev@shenfy.com"
    url = "https://github.com/shenfy/mineola"
    description = "Light-weight OpenGL ES Rendering Engine"
    topics = ("Computer Graphics", "Computer Vision", "Image Processing")
    settings = "os", "compiler", "build_type", "arch"
    requires = ("boost/1.69.0",
        "glm/0.9.8.5@bincrafters/stable",
        "imgpp/2.0.1@shenfy/testing",
        "fx_gltf/1.1.0@shenfy/testing",
        "stb/20200203",
        "glfw/3.3.2")
    options = {"shared": [False]}
    default_options = {"shared": False, "boost:header_only": True}
    generators = "cmake_paths", "cmake_find_package"
    exports_sources = "../src/*", "CMakeLists.txt", "include/*", "src/*", "resrc/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["mineola"]
