#include "prefix.h"
#include "../include/ImgppTextureSrc.h"

namespace mineola {

ImgppTextureSrc::ImgppTextureSrc(uint32_t faces, uint32_t levels, bool is_array, bool is_compressed)
  : faces_(faces), levels_(levels), is_array_(is_array), is_compressed_(is_compressed) {
}

const void *ImgppTextureSrc::Data(uint32_t layer, uint32_t face, uint32_t level) const {
  if (is_compressed_) {
    return bc_rois_[level].GetData();
  } else {
    return rois_[level].GetData();
  }
}

uint32_t ImgppTextureSrc::DataSize(uint32_t level) const {
  if (is_compressed_) {
    return bc_rois_[level].SlicePitch() * bc_rois_[level].Depth();
  } else {
    return rois_[level].SlicePitch() * rois_[level].Depth();
  }
}

glm::ivec4 ImgppTextureSrc::Dimensions(uint32_t level) const {
  uint32_t idx = level;
  glm::ivec4 dims;
  if (is_array_ || faces_ > 1) {
    if (is_compressed_) {
      dims = glm::ivec4(bc_rois_[level].Width(), bc_rois_[level].Height(), 1, bc_rois_[level].Depth());
    } else {
      dims = glm::ivec4(rois_[level].Width(), rois_[level].Height(), 1, rois_[level].Depth());
    }
  } else {
    if (is_compressed_) {
      dims = glm::ivec4(bc_rois_[level].Width(), bc_rois_[level].Height(), bc_rois_[level].Depth(), 1);
    } else {
      dims = glm::ivec4(rois_[level].Width(), rois_[level].Height(), rois_[level].Depth(), 1);
    }
  }
  return dims;
}

int ImgppTextureSrc::Alignment(uint32_t level) const {
  if (is_compressed_) {
    return 4;
  } else {
    const auto &roi = rois_[level];
    auto old_pitch = roi.Pitch();
    for (int i = 0; i < 4; ++i) {
      auto new_pitch = imgpp::ImgROI::CalcPitch(roi.Width(), roi.Channel(), roi.BPC(), 1 << i);
      if (new_pitch == old_pitch) {
        return 1 << i;
      }
    }
    return 1;  // not 1,2,4,8! should not happen!
  }
}

uint32_t ImgppTextureSrc::Levels() const {
  return levels_;
}

uint32_t ImgppTextureSrc::Faces() const {
  return faces_;
}

bool ImgppTextureSrc::IsArray() const {
  return is_array_;
}

bool ImgppTextureSrc::IsCompressed() const {
  return is_compressed_;
}

const imgpp::ImgROI& ImgppTextureSrc::ROI(uint32_t level) const {
  return rois_[0];
}

const imgpp::BCImgROI& ImgppTextureSrc::BCROI(uint32_t level) const {
  return bc_rois_[0];
}

void ImgppTextureSrc::AddBuffer(const imgpp::ImgBuffer &buffer) {
  buffers_.push_back(buffer);
}

void ImgppTextureSrc::AddROI(const imgpp::ImgROI &roi) {
  rois_.push_back(roi);
}

void ImgppTextureSrc::AddBCROI(const imgpp::BCImgROI &bc_roi) {
  bc_rois_.push_back(bc_roi);
}

}  // namespace
