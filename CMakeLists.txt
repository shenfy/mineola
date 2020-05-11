#Project information
cmake_minimum_required(VERSION 3.8)

project(mineola)

set(${PROJECT_NAME}_MAJOR_VERSION 1)
set(${PROJECT_NAME}_MINOR_VERSION 0)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

if (WIN32)
    include_directories("C:\\include")
    link_directories("C:\\lib")
else ()
    include_directories("/usr/local/include")
endif ()

file(GLOB MINEOLA_SRC src/*.cpp)
file(GLOB MINEOLA_HDR include/*.h)
add_library(mineola ${MINEOLA_SRC} ${MINEOLA_HDR})

if(WIN32)
  # Use this snippet *after* PROJECT(xxx):
  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX C:/ CACHE PATH "install to C:/ on windows" FORCE)
  ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(WIN32)

if (WIN32)
    install(TARGETS mineola CONFIGURATIONS Debug DESTINATION lib/Debug)
    install(TARGETS mineola CONFIGURATIONS Release DESTINATION lib/Release)
else ()
    install(TARGETS mineola DESTINATION lib)
endif ()
install(FILES ${MINEOLA_HDR} DESTINATION include/mineola)


# pc example
file(GLOB PC_EXAMPLE_SRC pcexample/src/example.cpp pcexample/src/AppFrame.cpp pcexample/src/glad.c)
add_executable(pcexample ${PC_EXAMPLE_SRC})
add_custom_command(
  TARGET pcexample
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pcexample/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")

if (WIN32)
  target_link_libraries(pcexample mineola)
else ()
  target_link_libraries(pcexample mineola glfw pthread)
endif ()

# gltf example
file(GLOB GLTF_SRC pcexample/src/gltfdemo.cpp pcexample/src/AppFrame.cpp pcexample/src/glad.c)
add_executable(gltfdemo ${GLTF_SRC})
if (WIN32)
  target_link_libraries(gltfdemo mineola)
else ()
  target_link_libraries(gltfdemo mineola glfw pthread)
endif ()

# imgpptexturesrc example
file(GLOB IMGPPTEX_SRC pcexample/src/imgpptexdemo.cpp pcexample/src/TextureMixer.cpp
  pcexample/src/AppFrame.cpp pcexample/src/glad.c)
add_executable(imgpptexdemo ${IMGPPTEX_SRC})
if (WIN32)
  target_link_libraries(imgpptexdemo mineola)
else ()
  target_link_libraries(imgpptexdemo mineola glfw pthread)
endif ()
