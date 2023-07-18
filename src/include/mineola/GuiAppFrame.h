#ifndef MINEOLA_GUIAPPFRAME_H
#define MINEOLA_GUIAPPFRAME_H

#include <stdint.h>
#include "GLMDefines.h"

struct GLFWwindow;

namespace mineola {

class GuiAppFrame {
public:
  GuiAppFrame();
  virtual ~GuiAppFrame();

  virtual void Run(uint32_t width, uint32_t height);

protected:
  virtual bool CreateRenderWindow();
  virtual bool InitScene();  // load default effect, etc.
  virtual void ReleaseScene();

  void InitImGui();
  void ImGuiPreRender();
  void ImGuiPostRender();
  void DestroyImGui();

protected:
  GLFWwindow *window_{ nullptr };
  uint32_t window_width_, window_height_;
  bool running_;
};
}

#endif
