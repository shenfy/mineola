cmake_minimum_required(VERSION 3.16)
project(mineola VERSION 1.3.0 LANGUAGES C CXX)

message(STATUS ${CMAKE_SYSTEM_NAME})

if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
  set(MINEOLA_iOS 1)
  message(STATUS "Target platform setting to iOS")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  set(MINEOLA_MacOS 1)
  message(STATUS "Target platform setting to MacOS")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  set(MINEOLA_Android 1)
  message(STATUS "Target platform setting to Android")
else ()
  set(MINEOLA_PC 1)
  set(MINEOLA_PC_HEADLESS NO CACHE BOOL "Use EGL headless build")
  message(STATUS "Target platform setting to PC")
endif()

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Decide the platform we're buiding for
message(STATUS "Building for ${CMAKE_SYSTEM_NAME}:${CMAKE_SYSTEM}")

# Find common dependencies
find_package(Boost REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Platform specific dependencies
if (MINEOLA_PC)
  find_package(OpenGL COMPONENTS OpenGL REQUIRED)
  # use GLX and EGL modules
  if (TARGET OpenGL::OpenGL)
    if (MINEOLA_PC_HEADLESS)
      find_package(OpenGL COMPONENTS EGL REQUIRED)
    else()
      find_package(OpenGL COMPONENTS GLX REQUIRED)
      find_package(glfw3 CONFIG REQUIRED)
    endif()
  endif()
endif()

add_subdirectory(src)

add_subdirectory(apps)

install(TARGETS mineola
  EXPORT mineolaTargets
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/mineola)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "mineolaConfigVersion.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(EXPORT mineolaTargets
  FILE mineolaTargets.cmake
  NAMESPACE mineola::
  DESTINATION lib/cmake/mineola)

install(FILES "mineolaConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/mineolaConfigVersion.cmake"
  DESTINATION lib/cmake/mineola)

set(CPACK_VERBATIM_VARIABLES True)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "dev@shenfy.com")
set(CPACK_GENERATOR "ZIP")
include(CPack)
