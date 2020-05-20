#ifndef MINEOLA_IMGPPTEXTURESRC_H
#define MINEOLA_IMGPPTEXTURESRC_H

#include <vector>
#include <imgpp/imgpp.hpp>
#include <imgpp/imgpp_bc.hpp>
#include "TextureDesc.h"

namespace mineola {

class ImgppTextureSrc : public TextureSrcData {
public:
  ImgppTextureSrc(uint32_t faces, uint32_t layers, uint32_t levels);
  // this implementation simply ignores layer and face
  const void *Data(uint32_t face, uint32_t layer, uint32_t level) const override;
  uint32_t DataSize(uint32_t level) const override;
  // {width, height, depth}
  glm::ivec3 Dimensions(uint32_t level) const override;
  int Alignment(uint32_t level) const override;

  uint32_t Levels() const;
  uint32_t Layers() const;
  uint32_t Faces() const;
  bool IsCompressed() const;
  const imgpp::ImgROI &ROI(uint32_t level) const;
  const imgpp::BCImgROI &BCROI(uint32_t level) const;

  void AddBuffer(const imgpp::ImgBuffer &buffer);
  void AddROI(const imgpp::ImgROI &roi);
  void AddBCROI(const imgpp::BCImgROI &bc_roi);

private:
  uint32_t CalcSliceID(uint32_t face, uint32_t layer) const;
  std::vector<imgpp::ImgBuffer> buffers_;
  std::vector<imgpp::ImgROI> rois_;
  std::vector<imgpp::BCImgROI> bc_rois_;

  uint32_t faces_{1};
  uint32_t layers_{1};
  uint32_t levels_{1};
};

}  // namespace

#endif
