cmake_minimum_required(VERSION 3.16)
project(mineola VERSION 1.1.0 LANGUAGES C CXX)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(imgpp REQUIRED)
find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(fx_gltf REQUIRED)
find_package(glfw REQUIRED)
find_package(stb REQUIRED)
find_package(OpenGL COMPONENTS OpenGL)

set(MINEOLA_SRC
  ../src/src/AnimatedEntity.cpp ../src/src/Animation.cpp ../src/src/AppHelper.cpp
  ../src/src/ArcballController.cpp ../src/src/BasisObj.cpp ../src/src/CameraController.cpp
  ../src/src/Camera.cpp ../src/src/Engine.cpp ../src/src/Entity.cpp ../src/src/FileSystem.cpp
  ../src/src/FPSController.cpp ../src/src/Framebuffer.cpp ../src/src/GLEffect.cpp
  ../src/src/GLMHelper.cpp ../src/src/GLProgram.cpp ../src/src/GLShader.cpp ../src/src/GLTFLoader.cpp
  ../src/src/GraphicsBuffer.cpp ../src/src/ImgppTextureSrc.cpp
  ../src/src/KTXImageLoader.cpp ../src/src/Light.cpp ../src/src/Material.cpp ../src/src/MeshIO.cpp
  ../src/src/PBRShaders.cpp ../src/src/PolygonSoup.cpp ../src/src/PolygonSoupLoader.cpp
  ../src/src/PolygonSoupSerialization.cpp ../src/src/PrimitiveHelper.cpp ../src/src/Rbt.cpp
  ../src/src/Renderable.cpp ../src/src/RenderPass.cpp ../src/src/RenderState.cpp
  ../src/src/RenderStateFactory.cpp ../src/src/RenderStateManager.cpp
  ../src/src/ResourceManager.cpp ../src/src/SceneLoader.cpp ../src/src/SceneNode.cpp
  ../src/src/ShaderParser.cpp ../src/src/Skin.cpp ../src/src/STBImagePlugin.cpp ../src/src/Texture.cpp
  ../src/src/TextureHelper.cpp ../src/src/TurntableController.cpp
  ../src/src/UniformBlock.cpp ../src/src/UniformHelper.cpp ../src/src/UniformWrappers.cpp
  ../src/src/VertexType.cpp ../src/src/Viewport.cpp)

add_library(mineola STATIC)
target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(mineola PUBLIC
  imgpp::imgpp
  Boost::Boost
  glm::glm
  fx_gltf::fx_gltf
  stb::stb
  ${CMAKE_DL_LIBS})

add_executable(gltfview)
target_sources(gltfview PRIVATE
  src/gltfdemo.cpp src/AppFrame.cpp src/glad.c)
target_link_libraries(gltfview PRIVATE mineola glfw::glfw)
add_custom_command(
  TARGET gltfview
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")

