cmake_minimum_required(VERSION 3.16)
project(mineola VERSION 1.1.0 LANGUAGES C CXX)

if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(imgpp REQUIRED)
find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(fx_gltf REQUIRED)
find_package(glfw REQUIRED)
find_package(OpenGL COMPONENTS OpenGL)

set(MINEOLA_SRC
  src/AnimatedEntity.cpp src/Animation.cpp src/AppHelper.cpp src/ArcballController.cpp src/BasisObj.cpp src/CameraController.cpp
  src/Camera.cpp src/Engine.cpp src/Entity.cpp src/FileSystem.cpp src/FPSController.cpp src/Framebuffer.cpp src/GLEffect.cpp
  src/GLMHelper.cpp src/GLProgram.cpp src/GLShader.cpp src/GLTFLoader.cpp src/GraphicsBuffer.cpp src/ImgppTextureSrc.cpp
  src/KTXImageLoader.cpp src/Light.cpp src/Material.cpp src/MeshIO.cpp src/PBRShaders.cpp src/PolygonSoup.cpp src/PolygonSoupLoader.cpp
  src/PolygonSoupSerialization.cpp src/PrimitiveHelper.cpp src/Rbt.cpp src/Renderable.cpp src/RenderPass.cpp src/RenderState.cpp
  src/RenderStateFactory.cpp src/RenderStateManager.cpp src/ResourceManager.cpp src/SceneLoader.cpp src/SceneNode.cpp
  src/ShaderParser.cpp src/Skin.cpp src/STBImagePlugin.cpp src/Texture.cpp src/TextureHelper.cpp src/TurntableController.cpp
  src/UniformBlock.cpp src/UniformHelper.cpp src/UniformWrappers.cpp src/VertexType.cpp src/Viewport.cpp)

add_library(mineola STATIC)
target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(mineola PUBLIC
  imgpp::imgpp
  Boost::Boost
  glm::glm
  fx_gltf::fx_gltf
  ${CMAKE_DL_LIBS})

add_executable(gltfview)
target_sources(gltfview PRIVATE
  pcexample/src/gltfdemo.cpp pcexample/src/AppFrame.cpp pcexample/src/glad.c)
target_link_libraries(gltfview PRIVATE mineola glfw::glfw)
add_custom_command(
  TARGET gltfview
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pcexample/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")

