# iOS example
if (MINEOLA_iOS)
  add_executable(gltfview MACOSX_BUNDLE)
  set_target_properties(gltfview PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.shenfy.mineola.gltfview
    MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION})
  target_sources(gltfview PRIVATE
    ios/AppDelegate.h ios/AppDelegate.m ios/RenderTest.h ios/RenderTest.mm ios/main.m)
  set_source_files_properties(pc/resrc/shadow.frag pc/resrc/shadow.vert pc/resrc/venice16.ktx ios/gltf.json pc/resrc/CesiumMan.glb
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  target_sources(gltfview PRIVATE
      pc/resrc/shadow.frag pc/resrc/shadow.vert pc/resrc/venice16.ktx ios/gltf.json pc/resrc/CesiumMan.glb)
  target_compile_features(gltfview PUBLIC cxx_std_17)
  target_link_libraries(gltfview PUBLIC
    mineola
    "-framework GLKit" "-framework OpenGLES"
    "-framework UIKit" "-framework Foundation")
endif()

# PC examples
if (MINEOLA_PC)
  if (MINEOLA_PC_HEADLESS)
    add_executable(egltest)
    target_sources(egltest PRIVATE pc/egltest.cpp)
    target_link_libraries(egltest PRIVATE mineola OpenGL::EGL)
    add_custom_command(
      TARGET egltest
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
      COMMENT "Copy resrc folder to build folder")
  else()
    add_executable(gltfview)
    target_sources(gltfview PRIVATE pc/gltfdemo.cpp)
    target_link_libraries(gltfview PRIVATE mineola OpenGL::OpenGL)
    add_custom_command(
      TARGET gltfview
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
      COMMENT "Copy resrc folder to build folder")

    add_executable(cloth)
    target_sources(cloth PRIVATE pc/cloth.cpp)
    target_link_libraries(cloth PRIVATE mineola OpenGL::OpenGL)
    add_custom_command(
      TARGET cloth
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
      COMMENT "Copy resrc folder to build folder")

    add_executable(ocean)
    target_sources(ocean PRIVATE pc/ocean.cpp)
    target_link_libraries(ocean PRIVATE mineola OpenGL::OpenGL)
    add_custom_command(
      TARGET ocean
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/pc/resrc ${CMAKE_CURRENT_BINARY_DIR}/resrc
      COMMENT "Copy resrc folder to build folder")

    add_executable(envlight)
    target_sources(envlight PRIVATE pc/envlight.cpp pc/EnvBRDF.cpp)
    target_link_libraries(envlight PRIVATE mineola)

  endif()
endif()
