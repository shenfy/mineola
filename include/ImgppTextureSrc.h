#ifndef MINEOLA_IMGPPTEXTURESRC_H
#define MINEOLA_IMGPPTEXTURESRC_H

#include <vector>
#include <imgpp/imgpp.hpp>
#include "TextureDesc.h"

namespace mineola {

class ImgppTextureSrc : public TextureSrcData {
public:
  // this implementation simply ignores layer and face
  const void *Data(uint32_t layer, uint32_t face, uint32_t level) const override;

  // simply returns 0. (only used in compressed textures)
  uint32_t DataSize(uint32_t level) const override;

  glm::ivec3 Dimensions(uint32_t level) const override;

  int Alignment(uint32_t level) const override;

  void AddBuffer(const imgpp::ImgBuffer &buffer);
  void AddROI(const imgpp::ImgROI &roi);

private:
  std::vector<imgpp::ImgBuffer> buffers_;
  std::vector<imgpp::ImgROI> images_;
};

}  // namespace

#endif