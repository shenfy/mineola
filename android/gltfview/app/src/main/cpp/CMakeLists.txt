# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../conan_build ${CMAKE_MODULE_PATH})

find_library(log-lib log)
find_package(mineola REQUIRED)

add_library(gltfviewer_lib SHARED)
target_sources(gltfviewer_lib PRIVATE GLTFViewer.cpp)
target_compile_features(gltfviewer_lib PUBLIC cxx_std_17)
target_link_libraries(gltfviewer_lib PUBLIC
  mineola::mineola
  ${log-lib} )
