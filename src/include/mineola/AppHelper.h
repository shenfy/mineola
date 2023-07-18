#ifndef MINEOLA_APPHELPER_H
#define MINEOLA_APPHELPER_H
#include <cstdint>
#include <functional>
#include "Engine.h"

namespace mineola {

  bool InitEngine();
  void StartEngine();
  void ReleaseEngine();
  void SetScreenFramebuffer(int fbo, uint32_t w, uint32_t h);
  void ResizeScreen(uint32_t w, uint32_t h);
  void AddSearchPath(const char *path);

  void RenderGUI();
  void FrameMove();
  void Render();

  Engine &GetEngine();

}

#endif /* MINEOLA_APPHELPER_H */
