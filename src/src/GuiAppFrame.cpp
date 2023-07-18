#include "prefix.h"
#include <mineola/GuiAppFrame.h>
#include <mineola/glutility.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <mineola/AppHelper.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <mineola/imgui_impl_opengl3.h>

namespace {
using namespace mineola;

void OnResize(GLFWwindow *window, int width, int height) {
	GetEngine().OnSize(width, height);
}

void AppOnKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (ImGui::GetIO().WantCaptureKeyboard) {  // stop imgui events passing through
    return;
  }

  if (action == GLFW_REPEAT)
    return;
  GetEngine().OnKey((uint32_t)key,
  	action == GLFW_PRESS ? Engine::BUTTON_DOWN : Engine::BUTTON_UP);
}

void AppOnMouseButton(GLFWwindow *window, int button, int action, int mods) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  double xpos = 0.0, ypos = 0.0;
  glfwGetCursorPos(window, &xpos, &ypos);
  GetEngine().OnMouseButton((uint8_t)button,
    action == GLFW_PRESS ? Engine::BUTTON_DOWN : Engine::BUTTON_UP,
    (int)(xpos + 0.5), (int)(ypos + 0.5));
}

void AppOnMouseMove(GLFWwindow *window, double x, double y) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }
  GetEngine().OnMouseMove((int)(x + 0.5), (int)(y + 0.5));
}

void AppOnMouseScroll(GLFWwindow *window, double x_offset, double y_offset) {
  GetEngine().OnMouseScroll((int)x_offset, (int)y_offset);
}
}

namespace mineola {

GuiAppFrame::GuiAppFrame()
  : window_width_(512),
  window_height_(512),
  running_(false) {}

GuiAppFrame::~GuiAppFrame() {
}

bool GuiAppFrame::CreateRenderWindow() {
  // Initialize GLFW
  if( !glfwInit() ) {
    printf("Failed to initialize GLFW!\n");
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

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

void GuiAppFrame::InitImGui() {
  // init imgui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window_, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
}

void GuiAppFrame::ImGuiPreRender() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void GuiAppFrame::ImGuiPostRender() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiAppFrame::DestroyImGui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GuiAppFrame::Run(uint32_t width, uint32_t height) {
  //create render context and window
  if (width != 0) window_width_ = width;
  if (height != 0) window_height_ = height;
  if (!CreateRenderWindow()) {
    printf("Error: failed to create GL window!\n");
    return;
  }

  // call AppHelper::InitEngine
  InitEngine();
  InitImGui();
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
    // render GUI
    ImGuiPreRender();
    RenderGUI();
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // call frame move before rendering
    FrameMove();
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // render scene
    Render();
    ///////////////////////////////////////////

    ImGuiPostRender();

    // Swap front and back rendering buffers
    glfwSwapBuffers(window_);

    // Pool events
    glfwPollEvents();

    // Check if ESC key was pressed or window was closed
    running_ = running_ && !glfwWindowShouldClose(window_) && !en.TerminationSignaled();
  }

  ReleaseScene();
  ReleaseEngine();
  DestroyImGui();
  glfwTerminate();
}

bool GuiAppFrame::InitScene() {
  return true;
}

void GuiAppFrame::ReleaseScene() {
}

}
