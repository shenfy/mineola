# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

file(GLOB SRC_CPP_FILES ../../src/*.cpp)
include_directories("/usr/local/include")

add_library( # Sets the name of the library.
        mineola
        SHARED
        ${SRC_CPP_FILES} ../../MineolaJNIAPI.cpp)

find_library(log-lib log)

set(distribution_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../dist)
target_link_libraries(mineola GLESv3 EGL android ${log-lib})
# shared library .so path
set_target_properties(mineola PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${distribution_dir}/${ANDROID_ABI}")
# static library .a path
set_target_properties(mineola PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${distribution_dir}/${ANDROID_ABI}")
