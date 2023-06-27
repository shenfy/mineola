# Header and source files
set(MINEOLA_SRC
  AnimatedEntity.cpp
  Animation.cpp
  AppHelper.cpp
  ArcballController.cpp
  BasisObj.cpp
  CameraController.cpp
  Camera.cpp
  Engine.cpp
  Entity.cpp
  EnvLight.cpp
  FileSystem.cpp
  FPSController.cpp
  Framebuffer.cpp
  GLEffect.cpp
  GLMHelper.cpp
  GLProgram.cpp
  GLShader.cpp
  GLTFLoader.cpp
  GraphicsBuffer.cpp
  ImgppTextureSrc.cpp
  Light.cpp
  Material.cpp
  MeshIO.cpp
  PBRShaders.cpp
  PolygonSoup.cpp
  PolygonSoupLoader.cpp
  PolygonSoupSerialization.cpp
  PrefabHelper.cpp
  PrimitiveHelper.cpp
  Rbt.cpp
  Renderable.cpp
  RenderPass.cpp
  RenderState.cpp
  RenderStateFactory.cpp
  RenderStateManager.cpp
  ResourceManager.cpp
  SceneLoader.cpp
  SceneNode.cpp
  ShaderParser.cpp
  Skin.cpp
  STBImagePlugin.cpp
  Texture.cpp
  TextureHelper.cpp
  TextureTypes.cpp
  TurntableController.cpp
  UniformBlock.cpp
  UniformHelper.cpp
  UniformWrappers.cpp
  VertexType.cpp
  Viewport.cpp
  AABB.cpp
  #imgpp
  imgpp/BmpImg.cpp
  imgpp/Bson.cpp
  imgpp/ImgppGLHelper.cpp
  imgpp/ImgppHelper.cpp
  imgpp/JpegImg.cpp
  imgpp/KTXImage.cpp
  imgpp/PFMImg.cpp
  imgpp/PNGImg.cpp
  imgpp/PPMImg.cpp
  imgpp/ImgppStringParser.h)

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
  include/mineola/GLMDefines.h
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
  include/mineola/AABB.h
  include/mineola/ImgppAlgorithms.hpp
  include/mineola/ImgppBlockImg.hpp
  include/mineola/ImgppCompositeImg.hpp
  include/mineola/ImgppGLHelper.hpp
  include/mineola/ImgppGLMTraits.hpp
  include/mineola/ImgppImgBase.hpp
  include/mineola/Imgpp.hpp
  include/mineola/ImgppLoadersExt.hpp
  include/mineola/ImgppLoaders.hpp
  include/mineola/ImgppOpenCV.hpp
  include/mineola/ImgppSampler.hpp
  include/mineola/ImgppTextureDesc.hpp
  include/mineola/ImgppTextureHelper.hpp
  include/mineola/ImgppTypeTraits.hpp)

# Platform specific source and header files
if (MINEOLA_iOS)
  # A RendererViewController is provided for iOS
  set(MINEOLA_SRC ${MINEOLA_SRC} RendererViewController.mm)
  set(MINEOLA_HDR ${MINEOLA_HDR} include/mineola/RendererViewController.h)
elseif (MINEOLA_Android)
  # JNI interface for Android
  set(MINEOLA_SRC ${MINEOLA_SRC} android/src/MineolaJNIAPI.cpp)
elseif (MINEOLA_PC)
  # glad used as the GL loader on PC
  if (MINEOLA_PC_HEADLESS)
    set(MINEOLA_SRC ${MINEOLA_SRC} glad.c ServerAppFrame.cpp)
    set(MINEOLA_HDR ${MINEOLA_HDR}
      include/mineola/glad.h
      include/mineola/khrplatform.h
      include/mineola/ServerAppFrame.h)
  else()
    set(MINEOLA_SRC ${MINEOLA_SRC} glad.c AppFrame.cpp)
    set(MINEOLA_HDR ${MINEOLA_HDR}
      include/mineola/glad.h
      include/mineola/khrplatform.h
      include/mineola/AppFrame.h)
  endif()
endif()


# The mineola library
add_library(mineola STATIC)

target_sources(mineola PRIVATE ${MINEOLA_SRC})
target_compile_features(mineola PUBLIC cxx_std_17)
set_property(TARGET mineola PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(mineola PUBLIC
  $<BUILD_INTERFACE:${mineola_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(mineola PUBLIC
  glm::glm
  PNG::PNG
  JPEG::JPEG
  nlohmann_json::nlohmann_json
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS})
set_target_properties(mineola PROPERTIES PUBLIC_HEADER "${MINEOLA_HDR}")

if (MINEOLA_iOS)
  # Get rid of iOS deprecation warnings
  target_compile_definitions(mineola PUBLIC GLES_SILENCE_DEPRECATION)
elseif (MINEOLA_PC)
  if (MINEOLA_PC_HEADLESS)
    target_link_libraries(mineola PUBLIC OpenGL::EGL)
  else()
    target_link_libraries(mineola PUBLIC glfw)
  endif()
endif()
