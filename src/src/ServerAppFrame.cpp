#include "prefix.h"
#include <iostream>
#include <mineola/ServerAppFrame.h>
#include <mineola/glutility.h>
#include <EGL/egl.h>
#include <mineola/AppHelper.h>

namespace mineola {

ServerAppFrame::ServerAppFrame()
  : window_width_(512),
  window_height_(512),
  running_(false),
  egl_display_(nullptr),
  egl_surface_(nullptr) {}

ServerAppFrame::~ServerAppFrame() {
}

bool ServerAppFrame::CreateRenderWindow() {
  // Initialize EGL
  static const EGLint config_attribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
  };

  const EGLint pbuffer_attribs[] = {
    EGL_WIDTH, (EGLint)window_width_,
    EGL_HEIGHT, (EGLint)window_height_,
    EGL_NONE
  };

  egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  std::cout << "display: " << egl_display_ << std::endl;

  EGLint major = 0, minor = 0;
  EGLBoolean egl_status = eglInitialize(egl_display_, &major, &minor);
  if (egl_status == EGL_FALSE) {
    std::cerr << "Failed to initialize EGL!" << std::endl;
    eglTerminate(egl_display_);
    return false;
  }

  EGLint num_configs = 0;
  EGLConfig egl_config;
  egl_status = eglChooseConfig(egl_display_, config_attribs, &egl_config, 1, &num_configs);
  if (egl_status == EGL_FALSE) {
    std::cerr << "Failed to choose EGL config!" << std::endl;
    eglTerminate(egl_display_);
    return false;
  }

  egl_surface_ = eglCreatePbufferSurface(egl_display_, egl_config, pbuffer_attribs);
  std::cout << "surface: " << egl_surface_ << std::endl;
  if (egl_surface_ == EGL_NO_SURFACE) {
    std::cerr << "Faield to create EGL surface!" << std::endl;
    eglTerminate(egl_display_);
    return false;
  }

  eglBindAPI(EGL_OPENGL_API);
  EGLContext context = eglCreateContext(egl_display_, egl_config, EGL_NO_CONTEXT, NULL);
  eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, context);

  // init glad
  if (!gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress)) {
    std::cerr << "Failed to initialize glad!" << std::endl;
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(egl_display_);
    return false;
  }

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  return true;
}

void ServerAppFrame::Run(uint32_t width, uint32_t height) {
  //create render context and window
  if (width != 0) window_width_ = width;
  if (height != 0) window_height_ = height;
  if (!CreateRenderWindow()) {
    std::cerr << "Error: failed to create GL window!" << std::endl;
  }

  // call AppHelper::InitEngine
  InitEngine();
  AddSearchPath(".");

  // virtual function to create scene
  InitScene();

  // call AppHelper::StartEngine
  StartEngine();

  SetScreenFramebuffer(0, window_width_, window_height_);
  ResizeScreen(window_width_, window_height_);
  auto &en = GetEngine();

  //main loop
  running_ = true;
  while( running_ ) {
    ///////////////////////////////////////////
    //call frame move before rendering
    FrameMove();
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    //render
    Render();
    ///////////////////////////////////////////

    // Swap front and back rendering buffers
    eglSwapBuffers(egl_display_, egl_surface_);

    // Check if ESC key was pressed or window was closed
    running_ = running_ && !en.TerminationSignaled();
  }

  ReleaseScene();
  ReleaseEngine();
  eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglTerminate(egl_display_);
}

void ServerAppFrame::InitScene() {
}

void ServerAppFrame::ReleaseScene() {
}

}
