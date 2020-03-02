# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#
#add_library( # Sets the name of the library.
#             native-lib
#
#             # Sets the library as a shared library.
#             SHARED
#
#             # Provides a relative path to your source file(s).
#             src/main/cpp/native-lib.cpp )

#
#find_library( # Sets the name of the path variable.
#              log-lib
#
#              # Specifies the name of the NDK library that
#              # you want CMake to locate.
#              log )


#target_link_libraries( # Specifies the target library.
#                       native-lib
#
#                       # Links the target library to the log library
#                       # included in the NDK.
#                       ${log-lib} )

set(libimport_dir ${CMAKE_SOURCE_DIR}/../../dist)
set(distribution_dir ${CMAKE_SOURCE_DIR}/../dist)
add_library(mineola SHARED IMPORTED)
set_target_properties(mineola PROPERTIES IMPORTED_LOCATION
        ${libimport_dir}/${ANDROID_ABI}/libmineola.so)

include_directories("/usr/local/include")
message(STATUS ${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../../include)
file(GLOB SRC_CPP_FILES src/main/cpp/*.cpp)
add_library(rendertest SHARED ${SRC_CPP_FILES})
find_library(log-lib log)
target_link_libraries(rendertest GLESv3 EGL android ${log-lib} mineola)

# shared library .so path
set_target_properties(rendertest PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${distribution_dir}/${ANDROID_ABI}")
# static library .a path
set_target_properties(rendertest PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${distribution_dir}/${ANDROID_ABI}")

