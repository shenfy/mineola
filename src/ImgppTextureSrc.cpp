#include "prefix.h"
#include "../include/ImgppTextureSrc.h"

namespace mineola {

const void *ImgppTextureSrc::Data(uint32_t layer, uint32_t face, uint32_t level) const {
  return images_[level].GetData();
}

uint32_t ImgppTextureSrc::DataSize(uint32_t level) const {
  return 0;
}

glm::ivec3 ImgppTextureSrc::Dimensions(uint32_t level) const {
  uint32_t idx = level;
  const imgpp::ImgROI &roi = images_[idx];
  return glm::ivec3(roi.Width(), roi.Height(), roi.Depth());
}

int ImgppTextureSrc::Alignment(uint32_t level) const {
  const auto &roi = images_[level];
  auto old_pitch = roi.Pitch();
  for (int i = 0; i < 4; ++i) {
    auto new_pitch = imgpp::ImgROI::CalcPitch(roi.Width(), roi.Channel(), roi.BPC(), 1 << i);
    if (new_pitch == old_pitch) {
      return 1 << i;
    }
  }
  return 1;  // not 1,2,4,8! should not happen!
}

void ImgppTextureSrc::AddBuffer(const imgpp::ImgBuffer &buffer) {
  buffers_.push_back(buffer);
}

void ImgppTextureSrc::AddROI(const imgpp::ImgROI &roi) {
  images_.push_back(roi);
}

}  // namespace