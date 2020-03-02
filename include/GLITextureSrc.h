#ifndef MINEOLA_GLITEXTURESRC_H
#define MINEOLA_GLITEXTURESRC_H

#include <gli/gli.hpp>
#include "TextureDesc.h"

namespace mineola {

class GLITextureSrc : public TextureSrcData {
public:
  // this implementation simply ignores layer and face
  const void *Data(uint32_t layer, uint32_t face, uint32_t level) const override;
  uint32_t DataSize(uint32_t level) const override;

  glm::ivec3 Dimensions(uint32_t level) const override;
  int Alignment(uint32_t level) const override;

  void SetGLITexture(const gli::texture &texture);

private:
  gli::texture gli_texture_;
};

}  // namespace

#endif