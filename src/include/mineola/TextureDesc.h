#ifndef MINEOLA_TEXTUREDESC_H
#define MINEOLA_TEXTUREDESC_H

#include <glm/glm.hpp>
#include "glutility.h"

namespace mineola {

class TextureSrcData {
public:
  virtual const void *Data(uint32_t face, uint32_t layer, uint32_t level) const = 0;
  virtual uint32_t DataSize(uint32_t level) const = 0;

  // returns (width, height, depth)
  virtual glm::ivec3 Dimensions(uint32_t level) const = 0;
  virtual int Alignment(uint32_t level) const = 0;
};

// TODO: handle swizzle
struct TextureDesc {
  bool compressed {false};  // is compressed texture
  uint32_t type; // e.g. GL_TEXTURE_2D, etc.
  uint32_t access;
  uint32_t array_size; // texture array
  uint32_t levels; // 0==automatic generate mipmaps
  uint32_t format; // e.g. GL_RGB, GL_R32F, etc.
  uint32_t internal_format; // e.g. GL_RED, GL_RGB, GL_DEPTH_COMPONENT, etc.
  uint32_t data_type; // e.g. GL_UNSIGNED_BYTE, GL_FLOAT, etc.
  uint32_t width, height, depth; // for empty texture ONLY! Will be overwritten if init_images are provided
  uint32_t min_filter, mag_filter;  // sampling method
  uint32_t wrap_s, wrap_t;
  uint32_t samples { 1 }; // number of samples
  bool depth_compare { false };  // true to do depth comparison for shadow sampler
  uint32_t depth_func { GL_LEQUAL };

  std::shared_ptr<TextureSrcData> src_data;
};

struct SubTextureDesc {
  uint32_t level {0};
  uint32_t x_offset {0};
  uint32_t y_offset {0};
  uint32_t width {0};
  uint32_t height {0};
  uint32_t alignment {1};
  uint32_t data_size {0};  // ONLY required for compressed textures
};

}  // namespace

#endif
