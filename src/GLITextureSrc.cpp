#include "prefix.h"
#include "../include/GLITextureSrc.h"

namespace mineola {

const void *GLITextureSrc::Data(uint32_t layer, uint32_t face, uint32_t level) const {
  return gli_texture_.data(layer, face, level);
}

uint32_t GLITextureSrc::DataSize(uint32_t level) const {
  return (uint32_t)gli_texture_.size(level);
}

glm::ivec3 GLITextureSrc::Dimensions(uint32_t level) const {
  return gli_texture_.extent(level);
}

int GLITextureSrc::Alignment(uint32_t level) const {
  return 4;
}

void GLITextureSrc::SetGLITexture(const gli::texture &texture) {
  gli_texture_ = texture;
}

}  // namespace
