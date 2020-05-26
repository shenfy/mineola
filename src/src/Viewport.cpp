#include "prefix.h"
#include "../include/Viewport.h"
#include "../include/glutility.h"
#include "../include/Engine.h"
#include "../include/UniformBlock.h"

namespace mineola {

Viewport::Viewport()
  : left_ratio(0.0f), bottom_ratio(0.0f),
  width_ratio(1.0f), height_ratio(1.0f),
  left(0), bottom(0), width(0), height(0),
  absolute_size(false) {
}

Viewport::~Viewport() {
}

void Viewport::OnSize(uint32_t w, uint32_t h) {
  if (!absolute_size) {
    left = uint32_t(w * left_ratio);
    bottom = uint32_t(h * bottom_ratio);
    width = uint32_t(w * width_ratio);
    height = uint32_t(h * height_ratio);
  }
}

void Viewport::Activate() {
  glViewport(left, bottom, width, height);
  auto builtin_ub = Engine::Instance().BuiltinUniformBlock().lock();
  if (builtin_ub) {
    float viewport_size[] = {(float)width, (float)height, 0.0f, 0.0f};
    builtin_ub->UpdateVariable("_viewport_size", viewport_size);
  }
}

}
