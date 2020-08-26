cmake_minimum_required(VERSION 3.16)
project(mineola_pc VERSION 1.1.0 LANGUAGES C CXX)

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
  src/AnimatedEntity.cpp src/Animation.cpp src/AppHelper.cpp
  src/ArcballController.cpp src/BasisObj.cpp src/CameraController.cpp
  src/Camera.cpp src/Engine.cpp src/Entity.cpp src/FileSystem.cpp
  src/FPSController.cpp src/Framebuffer.cpp src/GLEffect.cpp
  src/GLMHelper.cpp src/GLProgram.cpp src/GLShader.cpp src/GLTFLoader.cpp
  src/GraphicsBuffer.cpp src/ImgppTextureSrc.cpp
  src/KTXImageLoader.cpp src/Light.cpp src/Material.cpp src/MeshIO.cpp
  src/PBRShaders.cpp src/PolygonSoup.cpp src/PolygonSoupLoader.cpp
  src/PolygonSoupSerialization.cpp src/PrimitiveHelper.cpp src/Rbt.cpp
  src/Renderable.cpp src/RenderPass.cpp src/RenderState.cpp
  src/RenderStateFactory.cpp src/RenderStateManager.cpp
  src/ResourceManager.cpp src/SceneLoader.cpp src/SceneNode.cpp
  src/ShaderParser.cpp src/Skin.cpp src/STBImagePlugin.cpp src/Texture.cpp
  src/TextureHelper.cpp src/TurntableController.cpp
  src/UniformBlock.cpp src/UniformHelper.cpp src/UniformWrappers.cpp
  src/VertexType.cpp src/Viewport.cpp src/glad.c)

set(MINEOLA_HDR
  include/mineola/AnimatedEntity.h include/mineola/Animation.h include/mineola/AppHelper.h
  include/mineola/BasisObj.h include/mineola/CameraController.h
  include/mineola/Camera.h include/mineola/Engine.h include/mineola/Entity.h include/mineola/FileSystem.h
  include/mineola/Framebuffer.h include/mineola/glad.h include/mineola/GLEffect.h
  include/mineola/GLMHelper.h include/mineola/GLProgram.h include/mineola/GLShader.h include/mineola/GLTFLoader.h
  include/mineola/glutility.h include/mineola/GraphicsBuffer.h include/mineola/ImgppTextureSrc.h
  include/mineola/khrplatform.h include/mineola/KTXImageLoader.h include/mineola/Light.h include/mineola/ManagerBase.h
  include/mineola/Material.h include/mineola/MathHelper.h include/mineola/MeshIO.h include/mineola/Noncopyable.h
  include/mineola/PBRShaders.h include/mineola/PixelType.h include/mineola/PolygonSoup.h include/mineola/PolygonSoupLoader.h
  include/mineola/PolygonSoupSerialization.h include/mineola/PrimitiveHelper.h include/mineola/Rbt.h
  include/mineola/Renderable.h include/mineola/RenderPass.h include/mineola/RenderState.h
  include/mineola/RenderStateFactory.h include/mineola/RenderStateManager.h
  include/mineola/ResourceManager.h include/mineola/SceneLoader.h include/mineola/SceneNode.h
  include/mineola/ShaderParser.h include/mineola/Skin.h include/mineola/STBImagePlugin.h include/mineola/TextureDesc.h include/mineola/Texture.h
  include/mineola/TextureHelper.h include/mineola/TypeMapping.h
  include/mineola/UniformBlock.h include/mineola/UniformHelper.h include/mineola/UniformWrappers.h
  include/mineola/VertexType.h include/mineola/Viewport.h include/mineola/Visitor.h)

add_library(mineola STATIC)
target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(mineola PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(mineola PUBLIC
  imgpp::imgpp
  Boost::Boost
  glm::glm
  fx_gltf::fx_gltf
  stb::stb
  ${CMAKE_DL_LIBS})
set_target_properties(mineola PROPERTIES PUBLIC_HEADER "${MINEOLA_HDR}")
install(TARGETS mineola EXPORT mineolaTargets
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/mineola)

add_executable(gltfview)
target_sources(gltfview PRIVATE
  src/gltfdemo.cpp src/AppFrame.cpp)
target_link_libraries(gltfview PRIVATE mineola glfw::glfw)
add_custom_command(
  TARGET gltfview
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")

add_executable(cloth)
target_sources(cloth PRIVATE
  src/cloth.cpp src/AppFrame.cpp)
target_link_libraries(cloth PRIVATE mineola glfw::glfw)
add_custom_command(
  TARGET cloth
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")

add_executable(ocean)
target_sources(ocean PRIVATE
  src/ocean.cpp src/AppFrame.cpp)
target_link_libraries(ocean PRIVATE mineola glfw::glfw)
add_custom_command(
  TARGET ocean
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
  COMMENT "Copy resrc folder to build folder")
