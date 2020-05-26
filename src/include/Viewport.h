#ifndef MINEOLA_VIEWPORT_H
#define MINEOLA_VIEWPORT_H

#include <cstdint>

namespace mineola {

class Viewport {
public:
  Viewport();
  ~Viewport();

  void OnSize(uint32_t w, uint32_t h);
  void Activate();

  float left_ratio;
  float bottom_ratio;
  float width_ratio;
  float height_ratio;

  uint32_t left;
  uint32_t bottom;
  uint32_t width;
  uint32_t height;
  bool absolute_size;
};

}

#endif
