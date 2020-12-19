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
  ../src/src/Light.cpp ../src/src/Material.cpp ../src/src/MeshIO.cpp
  ../src/src/PBRShaders.cpp ../src/src/PolygonSoup.cpp ../src/src/PolygonSoupLoader.cpp
  ../src/src/PolygonSoupSerialization.cpp ../src/src/PrimitiveHelper.cpp ../src/src/Rbt.cpp
  ../src/src/Renderable.cpp ../src/src/RenderPass.cpp ../src/src/RenderState.cpp
  ../src/src/RenderStateFactory.cpp ../src/src/RenderStateManager.cpp
  ../src/src/ResourceManager.cpp ../src/src/SceneLoader.cpp ../src/src/SceneNode.cpp
  ../src/src/ShaderParser.cpp ../src/src/Skin.cpp ../src/src/STBImagePlugin.cpp ../src/src/Texture.cpp
  ../src/src/TextureHelper.cpp ../src/src/TextureTypes.cpp ../src/src/TurntableController.cpp
  ../src/src/UniformBlock.cpp ../src/src/UniformHelper.cpp ../src/src/UniformWrappers.cpp
  ../src/src/EnvLight.cpp ../src/src/PrefabHelper.cpp
  ../src/src/VertexType.cpp ../src/src/Viewport.cpp ../src/src/glad.c)

set(MINEOLA_HDR
  ../src/include/mineola/AnimatedEntity.h ../src/include/mineola/Animation.h ../src/include/mineola/AppHelper.h
  ../src/include/mineola/BasisObj.h ../src/include/mineola/CameraController.h
  ../src/include/mineola/Camera.h ../src/include/mineola/Engine.h ../src/include/mineola/Entity.h ../src/include/mineola/FileSystem.h
  ../src/include/mineola/Framebuffer.h ../src/include/mineola/glad.h ../src/include/mineola/GLEffect.h
  ../src/include/mineola/GLMHelper.h ../src/include/mineola/GLProgram.h ../src/include/mineola/GLShader.h ../src/include/mineola/GLTFLoader.h
  ../src/include/mineola/glutility.h ../src/include/mineola/GraphicsBuffer.h ../src/include/mineola/ImgppTextureSrc.h
  ../src/include/mineola/khrplatform.h ../src/include/mineola/Light.h ../src/include/mineola/ManagerBase.h
  ../src/include/mineola/Material.h ../src/include/mineola/MathHelper.h ../src/include/mineola/MeshIO.h ../src/include/mineola/Noncopyable.h
  ../src/include/mineola/PBRShaders.h ../src/include/mineola/PixelType.h ../src/include/mineola/PolygonSoup.h ../src/include/mineola/PolygonSoupLoader.h
  ../src/include/mineola/PolygonSoupSerialization.h ../src/include/mineola/PrimitiveHelper.h ../src/include/mineola/Rbt.h
  ../src/include/mineola/Renderable.h ../src/include/mineola/RenderPass.h ../src/include/mineola/RenderState.h
  ../src/include/mineola/RenderStateFactory.h ../src/include/mineola/RenderStateManager.h
  ../src/include/mineola/ResourceManager.h ../src/include/mineola/SceneLoader.h ../src/include/mineola/SceneNode.h
  ../src/include/mineola/ShaderParser.h ../src/include/mineola/Skin.h ../src/include/mineola/STBImagePlugin.h ../src/include/mineola/TextureDesc.h ../src/include/mineola/Texture.h
  ../src/include/mineola/TextureHelper.h ../src/include/mineola/TextureTypes.h ../src/include/mineola/TypeMapping.h
  ../src/include/mineola/UniformBlock.h ../src/include/mineola/UniformHelper.h ../src/include/mineola/UniformWrappers.h
  ../src/include/mineola/EnvLight.h ../src/include/mineola/PrefabHelper.h ../src/include/mineola/SH3.h
  ../src/include/mineola/ReservedTextureUnits.h
  ../src/include/mineola/VertexType.h ../src/include/mineola/Viewport.h ../src/include/mineola/Visitor.h)

add_library(mineola STATIC)
target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(mineola PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/include>
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
