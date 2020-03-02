#ifndef MINEOLA_APPFRAME_H
#define MINEOLA_APPFRAME_H

#include <stdint.h>

struct GLFWwindow;

namespace mineola {

class AppFrame {
public:
  AppFrame();
  virtual ~AppFrame();

  virtual void Run(uint32_t width, uint32_t height);

protected:
  virtual bool CreateRenderWindow();
  virtual bool InitScene(); //load default effect, etc.
  virtual void ReleaseScene();

protected:
  GLFWwindow *window_{ nullptr };
  uint32_t window_width_, window_height_;
  bool running_;
};
}

#endif
