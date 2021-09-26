from conans import ConanFile, CMake, tools
import os, pathlib

class MineolaConan(ConanFile):
    name = "mineola"
    version = "1.2.1"
    license = "MIT"
    author = "Fangyang Shen dev@shenfy.com"
    url = "https://github.com/shenfy/mineola"
    description = "Light-weight OpenGL ES Rendering Engine"
    topics = ("Computer Graphics", "Computer Vision", "Image Processing")
    settings = "os", "compiler", "build_type", "arch"

    options = {"shared": [False]}
    default_options = {"shared": False, "boost:header_only": True, "imgpp:no_ext_libs": True}
    generators = "cmake_paths", "cmake_find_package"
    exports_sources = "src/*"

    def requirements(self):
        self.requires("boost/1.69.0")
        self.requires("glm/0.9.8.5@g-truc/stable")
        self.requires("imgpp/2.1.5@shenfy/testing")
        self.requires("fx_gltf/1.1.0@shenfy/testing")
        self.requires("stb/20200203")
        if self.settings.os != 'iOS' and self.settings.os != 'Android':
            # We use glfw for window creation on PC
            self.requires("glfw/3.3.2")

    def _configure(self):
        cmake = CMake(self)
        if self.settings.os == 'Android':
            if 'NDK' in os.environ:
                cmake.definitions['CMAKE_TOOLCHAIN_FILE'] = os.environ['NDK']
            cmake.definitions['ANDROID_STL'] = 'c++_static'
        cmake.configure(source_folder="src")
        return cmake

    def build(self):
        cmake = self._configure()
        cmake.build()

    def package(self):
        cmake = self._configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["mineola"]
