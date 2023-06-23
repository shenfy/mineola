#ifndef MINEOLA_SERVERAPPFRAME_H
#define MINEOLA_SERVERAPPFRAME_H

#include <stdint.h>

typedef void *EGLDisplay;
typedef void *EGLSurface;

namespace mineola {

class ServerAppFrame {
public:
  ServerAppFrame();
  virtual ~ServerAppFrame();

  virtual void Run(uint32_t width, uint32_t height);

protected:
  virtual bool CreateRenderWindow();
  virtual void InitScene(); //load default effect, etc.
  virtual void ReleaseScene();

protected:
  uint32_t window_width_, window_height_;
  bool running_;
  EGLDisplay egl_display_;
  EGLSurface egl_surface_;
};
}

#endif
