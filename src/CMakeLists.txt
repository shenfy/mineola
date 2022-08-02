cmake_minimum_required(VERSION 3.16)
project(mineola VERSION 1.2.3 LANGUAGES C CXX)

# Conan boilerplates
if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Decide the platform we're buiding for
message(STATUS "Building for ${CMAKE_SYSTEM_NAME}:${CMAKE_SYSTEM}")

if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
  set(MINEOLA_iOS 1)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
  set(MINEOLA_Android 1)
else ()
  set(MINEOLA_PC 1)
endif()

if (MINEOLA_iOS)
  message(STATUS "Platform choosed as iOS")
endif()
if (MINEOLA_Android)
  message(STATUS "Platform choosed as Android")
endif()
if (MINEOLA_PC)
  message(STATUS "Platform choosed as PC")
endif()

# Find dependencies
find_package(imgpp REQUIRED)
find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(fx_gltf REQUIRED)
find_package(stb REQUIRED)

# Collect header and source files
set(MINEOLA_SRC
  src/AnimatedEntity.cpp
  src/Animation.cpp
  src/AppHelper.cpp
  src/ArcballController.cpp
  src/BasisObj.cpp
  src/CameraController.cpp
  src/Camera.cpp
  src/Engine.cpp
  src/Entity.cpp
  src/EnvLight.cpp
  src/FileSystem.cpp
  src/FPSController.cpp
  src/Framebuffer.cpp
  src/GLEffect.cpp
  src/GLMHelper.cpp
  src/GLProgram.cpp
  src/GLShader.cpp
  src/GLTFLoader.cpp
  src/GraphicsBuffer.cpp
  src/ImgppTextureSrc.cpp
  src/Light.cpp
  src/Material.cpp
  src/MeshIO.cpp
  src/PBRShaders.cpp
  src/PolygonSoup.cpp
  src/PolygonSoupLoader.cpp
  src/PolygonSoupSerialization.cpp
  src/PrefabHelper.cpp
  src/PrimitiveHelper.cpp
  src/Rbt.cpp
  src/Renderable.cpp
  src/RenderPass.cpp
  src/RenderState.cpp
  src/RenderStateFactory.cpp
  src/RenderStateManager.cpp
  src/ResourceManager.cpp
  src/SceneLoader.cpp
  src/SceneNode.cpp
  src/ShaderParser.cpp
  src/Skin.cpp
  src/STBImagePlugin.cpp
  src/Texture.cpp
  src/TextureHelper.cpp
  src/TextureTypes.cpp
  src/TurntableController.cpp
  src/UniformBlock.cpp
  src/UniformHelper.cpp
  src/UniformWrappers.cpp
  src/VertexType.cpp
  src/Viewport.cpp
  src/AABB.cpp)

set(MINEOLA_HDR
  include/mineola/AnimatedEntity.h
  include/mineola/Animation.h
  include/mineola/AppHelper.h
  include/mineola/BasisObj.h
  include/mineola/CameraController.h
  include/mineola/Camera.h
  include/mineola/Engine.h
  include/mineola/Entity.h
  include/mineola/EnvLight.h
  include/mineola/FileSystem.h
  include/mineola/Framebuffer.h
  include/mineola/GLEffect.h
  include/mineola/GLMHelper.h
  include/mineola/GLProgram.h
  include/mineola/GLShader.h
  include/mineola/GLTFLoader.h
  include/mineola/glutility.h
  include/mineola/GraphicsBuffer.h
  include/mineola/ImgppTextureSrc.h
  include/mineola/Light.h
  include/mineola/ManagerBase.h
  include/mineola/Material.h
  include/mineola/MathHelper.h
  include/mineola/MeshIO.h
  include/mineola/Noncopyable.h
  include/mineola/PBRShaders.h
  include/mineola/PixelType.h
  include/mineola/PolygonSoup.h
  include/mineola/PolygonSoupLoader.h
  include/mineola/PolygonSoupSerialization.h
  include/mineola/PrefabHelper.h
  include/mineola/PrimitiveHelper.h
  include/mineola/Rbt.h
  include/mineola/Renderable.h
  include/mineola/RenderPass.h
  include/mineola/RenderState.h
  include/mineola/RenderStateFactory.h
  include/mineola/RenderStateManager.h
  include/mineola/ReservedTextureUnits.h
  include/mineola/ResourceManager.h
  include/mineola/SceneLoader.h
  include/mineola/SceneNode.h
  include/mineola/SH3.h
  include/mineola/ShaderParser.h
  include/mineola/Skin.h
  include/mineola/STBImagePlugin.h
  include/mineola/TextureDesc.h
  include/mineola/Texture.h
  include/mineola/TextureHelper.h
  include/mineola/TextureTypes.h
  include/mineola/TypeMapping.h
  include/mineola/UniformBlock.h
  include/mineola/UniformHelper.h
  include/mineola/UniformWrappers.h
  include/mineola/VertexType.h
  include/mineola/Viewport.h
  include/mineola/Visitor.h
  include/mineola/AABB.h)

if (MINEOLA_iOS)
  # We provide a RendererViewController on iOS
  set(MINEOLA_SRC ${MINEOLA_SRC} src/RendererViewController.mm)
  set(MINEOLA_HDR ${MINEOLA_HDR} include/mineola/RendererViewController.h)
endif()

if (MINEOLA_Android)
  # The Android JNI interface
  set(MINEOLA_SRC ${MINEOLA_SRC} android/src/MineolaJNIAPI.cpp)
endif()

if (MINEOLA_PC)
  # We're using glad as the GL loader on PC
  set(MINEOLA_SRC ${MINEOLA_SRC} src/glad.c src/AppFrame.cpp)
  set(MINEOLA_HDR ${MINEOLA_HDR} include/mineola/glad.h include/mineola/khrplatform.h include/mineola/AppFrame.h)

  find_package(glfw REQUIRED)
endif()

# The mineola library
add_library(mineola STATIC)
if (MINEOLA_iOS)
  # Get rid of iOS deprecation warnings
  target_compile_definitions(mineola PUBLIC GLES_SILENCE_DEPRECATION)
endif()
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
if (MINEOLA_PC)
  target_link_libraries(mineola PUBLIC glfw::glfw)
endif()
set_target_properties(mineola PROPERTIES PUBLIC_HEADER "${MINEOLA_HDR}")
install(TARGETS mineola EXPORT mineolaTargets
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/mineola)

# iOS example
if (MINEOLA_iOS)
  add_executable(gltfview MACOSX_BUNDLE)
  set_target_properties(gltfview PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.shenfy.mineola.gltfview
    MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION})
  target_sources(gltfview PRIVATE
    ios/src/AppDelegate.h ios/src/AppDelegate.m ios/src/RenderTest.h ios/src/RenderTest.mm ios/src/main.m)
  set_source_files_properties(pc/resrc/shadow.frag pc/resrc/shadow.vert pc/resrc/venice16.ktx ios/src/gltf.json pc/resrc/CesiumMan.glb
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(gltfview PRIVATE
      pc/resrc/shadow.frag pc/resrc/shadow.vert pc/resrc/venice16.ktx ios/src/gltf.json pc/resrc/CesiumMan.glb)
  target_compile_features(gltfview PUBLIC cxx_std_17)
  target_link_libraries(gltfview PUBLIC
    mineola
    "-framework GLKit" "-framework OpenGLES"
    "-framework UIKit" "-framework Foundation")
endif()

# PC examples
if (MINEOLA_PC)
  add_executable(gltfview)
  target_sources(gltfview PRIVATE
    pc/src/gltfdemo.cpp)
  target_link_libraries(gltfview PRIVATE mineola)
  add_custom_command(
    TARGET gltfview
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
    COMMENT "Copy resrc folder to build folder")

  add_executable(cloth)
  target_sources(cloth PRIVATE
    pc/src/cloth.cpp)
  target_link_libraries(cloth PRIVATE mineola)
  add_custom_command(
    TARGET cloth
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
    COMMENT "Copy resrc folder to build folder")

  add_executable(ocean)
  target_sources(ocean PRIVATE
    pc/src/ocean.cpp)
  target_link_libraries(ocean PRIVATE mineola)
  add_custom_command(
    TARGET ocean
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
    COMMENT "Copy resrc folder to build folder")
endif()
