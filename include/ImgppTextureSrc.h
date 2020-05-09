#ifndef MINEOLA_IMGPPTEXTURESRC_H
#define MINEOLA_IMGPPTEXTURESRC_H

#include <vector>
#include <imgpp/imgpp.hpp>
#include <imgpp/bcimgpp.hpp>
#include "TextureDesc.h"

namespace mineola {

class ImgppTextureSrc : public TextureSrcData {
public:
  ImgppTextureSrc(uint32_t faces, uint32_t levels, bool is_array, bool is_compressed);
  // this implementation simply ignores layer and face
  const void *Data(uint32_t layer, uint32_t face, uint32_t level) const override;
  uint32_t DataSize(uint32_t level) const override;
  // {width, height, depth, number_of_array}
  glm::ivec4 Dimensions(uint32_t level) const override;
  int Alignment(uint32_t level) const override;

  uint32_t Levels() const;
  uint32_t Faces() const;
  bool IsArray() const;
  bool IsCompressed() const;
  const imgpp::ImgROI& ROI(uint32_t level) const;
  const imgpp::BCImgROI& BCROI(uint32_t level) const;

  void AddBuffer(const imgpp::ImgBuffer &buffer);
  void AddROI(const imgpp::ImgROI &roi);
  void AddBCROI(const imgpp::BCImgROI &bc_roi);

private:
  std::vector<imgpp::ImgBuffer> buffers_;
  std::vector<imgpp::ImgROI> rois_;
  std::vector<imgpp::BCImgROI> bc_rois_;

  bool is_array_{false};
  bool is_compressed_{false};
  uint32_t levels_{1};
  uint32_t faces_{1};
};

}  // namespace

#endif
