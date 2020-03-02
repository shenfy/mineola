#ifndef MINEOLA_COMPRESSEDTEXTURESRC_H
#define MINEOLA_COMPRESSEDTEXTURESRC_H

#include <vector>
#include "TextureDesc.h"

namespace mineola {
class CompressedTextureSrc : public TextureSrcData {
public:
  const void *Data(uint32_t layer, uint32_t face, uint32_t level) const override;
  uint32_t DataSize(uint32_t level) const override;
  glm::ivec3 Dimensions(uint32_t level) const override;
  int Alignment(uint32_t level) const override;

  void SetTexture(
    uint32_t layers, uint32_t faces, uint32_t levels,
    uint32_t width, uint32_t height, uint32_t depth,
    std::vector<std::vector<char>> buffers);

private:
  std::vector<std::vector<char>> buffer_;
  std::vector<glm::ivec3> dimensions_;
  uint32_t layers_;
  uint32_t faces_;
  uint32_t levels_;
  uint32_t CalcLevelIdx(uint32_t layer, uint32_t face, uint32_t level) const;
};
}
#endif
