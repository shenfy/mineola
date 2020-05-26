#include "../include/AppFrame.h"
#include <mineola/glutility.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <mineola/AppHelper.h>

namespace {
using namespace mineola;

void OnResize(GLFWwindow *window, int width, int height) {
	GetEngine().OnSize(width, height);
}

void AppOnKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_REPEAT)
    return;
  GetEngine().OnKey((uint32_t)key,
  	action == GLFW_PRESS ? Engine::BUTTON_DOWN : Engine::BUTTON_UP);
}

void AppOnMouseButton(GLFWwindow *window, int button, int action, int mods) {
  double xpos = 0.0, ypos = 0.0;
  glfwGetCursorPos(window, &xpos, &ypos);
  GetEngine().OnMouseButton((uint8_t)button,
    action == GLFW_PRESS ? Engine::BUTTON_DOWN : Engine::BUTTON_UP,
    (int)(xpos + 0.5), (int)(ypos + 0.5));
}

void AppOnMouseMove(GLFWwindow *window, double x, double y) {
  GetEngine().OnMouseMove((int)(x + 0.5), (int)(y + 0.5));
}

void AppOnMouseScroll(GLFWwindow *window, double x_offset, double y_offset) {
  GetEngine().OnMouseScroll((int)x_offset, (int)y_offset);
}
}

namespace mineola {

AppFrame::AppFrame()
  : window_width_(512),
  window_height_(512),
  running_(false) {}

AppFrame::~AppFrame() {
}

bool AppFrame::CreateRenderWindow() {
  // Initialize GLFW
  if( !glfwInit() ) {
    printf("Failed to initialize GLFW!\n");
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create an OpenGL window
  window_ = glfwCreateWindow(window_width_, window_height_, "Render", NULL, NULL);
  if (!window_) {
    glfwTerminate();
    return false;
  }

  // select GL context
  glfwMakeContextCurrent(window_);

  // init glad
  if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize glad!\n");
    glfwTerminate();
    return false;
  }

  glfwSetFramebufferSizeCallback(window_, OnResize);
  glfwSetKeyCallback(window_, AppOnKey);
  glfwSetMouseButtonCallback(window_, AppOnMouseButton);
  glfwSetCursorPosCallback(window_, AppOnMouseMove);
  glfwSetScrollCallback(window_, AppOnMouseScroll);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // update window width and height to handle non 1:1 framebuffer to pixel ratios
  int fb_width = 0, fb_height = 0;
  glfwGetFramebufferSize(window_, &fb_width, &fb_height);
  window_width_ = (uint32_t)fb_width;
  window_height_ = (uint32_t)fb_height;
  return true;
}

void AppFrame::Run(uint32_t width, uint32_t height) {
  //create render context and window
  if (width != 0) window_width_ = width;
  if (height != 0) window_height_ = height;
  if (!CreateRenderWindow()) {
    printf("Error: failed to create GL window!\n");
    return;
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
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Render();
    ///////////////////////////////////////////

    // Swap front and back rendering buffers
    glfwSwapBuffers(window_);

    // Pool events
    glfwPollEvents();

    // Check if ESC key was pressed or window was closed
    running_ = running_ && !glfwWindowShouldClose(window_) && !en.TerminationSignaled();
  }

  ReleaseScene();
  ReleaseEngine();
  glfwTerminate();
}

bool AppFrame::InitScene() {
  return true;
}

void AppFrame::ReleaseScene() {
}

}
