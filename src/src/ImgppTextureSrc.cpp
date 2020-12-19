#include "prefix.h"
#include <mineola/ImgppTextureSrc.h>

namespace mineola {

ImgppTextureSrc::ImgppTextureSrc(
  uint32_t faces, uint32_t layers, uint32_t levels, imgpp::TextureFormat format)
  : faces_(faces), layers_(layers), levels_(levels), format_(format) {
  if (IsCompressed()) {
    rois_ = std::vector<imgpp::BlockImgROI>();
  } else {
    rois_ = std::vector<imgpp::ImgROI>();
  }
}

const void *ImgppTextureSrc::Data(uint32_t face, uint32_t layer, uint32_t level) const {
  uint32_t z = CalcSliceID(face, layer);
  if (IsCompressed()) {
    return std::get<std::vector<imgpp::BlockImgROI>>(rois_)[level].BlockAt(0, 0, z);
  } else {
    return std::get<std::vector<imgpp::ImgROI>>(rois_)[level].PtrAt(0, 0, z, 0);
  }
}

uint32_t ImgppTextureSrc::DataSize(uint32_t level) const {
  if (IsCompressed()) {
    auto &rois = std::get<std::vector<imgpp::BlockImgROI>>(rois_);
    return rois[level].SlicePitch() * rois[level].Depth();
  } else {
    auto &rois = std::get<std::vector<imgpp::ImgROI>>(rois_);
    return rois[level].SlicePitch() * rois[level].Depth();
  }
}

glm::ivec3 ImgppTextureSrc::Dimensions(uint32_t level) const {
  glm::ivec3 dims;
  if (IsCompressed()) {
    auto &rois = std::get<std::vector<imgpp::BlockImgROI>>(rois_);
    dims = glm::ivec3(rois[level].Width(), rois[level].Height(), rois[level].Depth());
  } else {
    auto &rois = std::get<std::vector<imgpp::ImgROI>>(rois_);
    dims = glm::ivec3(rois[level].Width(), rois[level].Height(), rois[level].Depth());
  }
  return dims;
}

int ImgppTextureSrc::Alignment(uint32_t level) const {
  if (IsCompressed()) {
    return 4;
  } else {
    auto &rois = std::get<std::vector<imgpp::ImgROI>>(rois_);
    const auto &roi = rois[level];
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

imgpp::TextureFormat ImgppTextureSrc::Format() const {
  return format_;
}

bool ImgppTextureSrc::IsCompressed() const {
  return imgpp::IsCompressedFormat(format_);
}

const imgpp::ImgROI &ImgppTextureSrc::ROI(uint32_t level) const {
  return std::get<std::vector<imgpp::ImgROI>>(rois_)[level];
}

const imgpp::BlockImgROI &ImgppTextureSrc::BCROI(uint32_t level) const {
  return std::get<std::vector<imgpp::BlockImgROI>>(rois_)[level];
}

void ImgppTextureSrc::AddBuffer(const imgpp::ImgBuffer &buffer) {
  buffers_.push_back(buffer);
}

void ImgppTextureSrc::AddROI(const imgpp::ImgROI &roi) {
  std::get<std::vector<imgpp::ImgROI>>(rois_).push_back(roi);
}

void ImgppTextureSrc::AddBCROI(const imgpp::BlockImgROI &bc_roi) {
  std::get<std::vector<imgpp::BlockImgROI>>(rois_).push_back(bc_roi);
}

uint32_t ImgppTextureSrc::CalcSliceID(uint32_t face, uint32_t layer) const {
  return face + layer * faces_;
}
}  // namespace
