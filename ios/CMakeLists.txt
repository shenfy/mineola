cmake_minimum_required(VERSION 3.16)
project(mineola_ios VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Boost REQUIRED)
find_package(imgpp REQUIRED)
find_package(glm REQUIRED)
find_package(fx_gltf REQUIRED)
find_package(stb REQUIRED)

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
  ../src/src/RendererViewController.mm
  ../src/src/RenderStateFactory.cpp ../src/src/RenderStateManager.cpp
  ../src/src/ResourceManager.cpp ../src/src/SceneLoader.cpp ../src/src/SceneNode.cpp
  ../src/src/ShaderParser.cpp ../src/src/Skin.cpp ../src/src/STBImagePlugin.cpp ../src/src/Texture.cpp
  ../src/src/TextureHelper.cpp ../src/src/TextureTypes.cpp ../src/src/TurntableController.cpp
  ../src/src/UniformBlock.cpp ../src/src/UniformHelper.cpp ../src/src/UniformWrappers.cpp
  ../src/src/EnvLight.cpp ../src/src/PrefabHelper.cpp
  ../src/src/VertexType.cpp ../src/src/Viewport.cpp)

add_library(mineola STATIC)
target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(mineola PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(mineola PUBLIC
  Boost::Boost
  imgpp::imgpp
  glm::glm
  fx_gltf::fx_gltf
  stb::stb
  ${CMAKE_DL_LIBS})


add_executable(gltfview MACOSX_BUNDLE)
set_target_properties(gltfview PROPERTIES
  MACOSX_BUNDLE_GUI_IDENTIFIER com.shenfy.mineola.gltfview
  MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION})
target_sources(gltfview PRIVATE
  src/AppDelegate.h src/AppDelegate.m src/RenderTest.h src/RenderTest.mm src/main.m)
set_source_files_properties(../pc/resrc/venice16.ktx src/gltf.json ../pc/resrc/CesiumMan.glb
  PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
target_sources(gltfview PRIVATE
  ../pc/resrc/venice16.ktx src/gltf.json ../pc/resrc/CesiumMan.glb)
target_compile_features(gltfview PUBLIC cxx_std_17)
target_link_libraries(gltfview PUBLIC
  mineola
  "-framework GLKit" "-framework OpenGLES"
  "-framework UIKit" "-framework Foundation")

