#include "prefix.h"
#include <mineola/ImgppTextureSrc.h>

namespace mineola {

ImgppTextureSrc::ImgppTextureSrc(
  uint32_t faces, uint32_t layers, uint32_t levels)
  : faces_(faces), layers_(layers), levels_(levels) {
}

const void *ImgppTextureSrc::Data(uint32_t face, uint32_t layer, uint32_t level) const {
  uint32_t z = CalcSliceID(face, layer);
  if (IsCompressed()) {
    return bc_rois_[level].BlockAt(0, 0, z);
  } else {
    return rois_[level].PtrAt(0, 0, z, 0);
  }
}

uint32_t ImgppTextureSrc::DataSize(uint32_t level) const {
  if (IsCompressed()) {
    return bc_rois_[level].SlicePitch() * bc_rois_[level].Depth();
  } else {
    return rois_[level].SlicePitch() * rois_[level].Depth();
  }
}

glm::ivec3 ImgppTextureSrc::Dimensions(uint32_t level) const {
  glm::ivec3 dims;
  if (IsCompressed()) {
    dims = glm::ivec3(bc_rois_[level].Width(), bc_rois_[level].Height(), bc_rois_[level].Depth());
  } else {
    dims = glm::ivec3(rois_[level].Width(), rois_[level].Height(), rois_[level].Depth());
  }
  return dims;
}

int ImgppTextureSrc::Alignment(uint32_t level) const {
  if (IsCompressed()) {
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

uint32_t ImgppTextureSrc::Layers() const {
  return layers_;
}

uint32_t ImgppTextureSrc::Faces() const {
  return faces_;
}

bool ImgppTextureSrc::IsCompressed() const {
  return !bc_rois_.empty();
}

const imgpp::ImgROI &ImgppTextureSrc::ROI(uint32_t level) const {
  return rois_[0];
}

const imgpp::BCImgROI &ImgppTextureSrc::BCROI(uint32_t level) const {
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

uint32_t ImgppTextureSrc::CalcSliceID(uint32_t face, uint32_t layer) const {
  return face + layer * faces_;
}
}  // namespace
