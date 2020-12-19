from conans import ConanFile, CMake, tools
import os, pathlib

class MineolaConan(ConanFile):
    name = "mineola"
    version = "1.1.0"
    license = "MIT"
    author = "Fangyang Shen dev@shenfy.com"
    url = "https://github.com/shenfy/mineola"
    description = "Light-weight OpenGL ES Rendering Engine"
    topics = ("Computer Graphics", "Computer Vision", "Image Processing")
    settings = "os", "compiler", "build_type", "arch"
    requires = ("boost/1.69.0",
        "glm/0.9.8.5@bincrafters/stable",
        "imgpp/2.1.4@shenfy/testing",
        "fx_gltf/1.1.0@shenfy/testing",
        "stb/20200203",
        "glfw/3.3.2")
    options = {"shared": [False]}
    default_options = {"shared": False, "boost:header_only": True, "imgpp:no_ext_libs": True}
    generators = "cmake_paths", "cmake_find_package"

    _pc_subfolder = 'pc'

    def export_sources(self):
        self.copy('../src/*', dst='src')
        self.copy('CMakeLists.txt', dst=self._pc_subfolder)
        self.copy('include/*', dst=self._pc_subfolder)
        self.copy('src/*', dst=self._pc_subfolder)
        self.copy('resrc/*', dst=self._pc_subfolder)

    def _configure(self):
        cmake = CMake(self)
        if (pathlib.Path(self.source_folder) / 'CMakeLists.txt').exists():
            # this is true when building inside the repository
            cmake.configure()
        else:
            # this is true when sources are exported
            cmake.configure(source_folder=self._pc_subfolder)
        return cmake

    def build(self):
        cmake = self._configure()
        cmake.build()

    def package(self):
        cmake = self._configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["mineola"]
