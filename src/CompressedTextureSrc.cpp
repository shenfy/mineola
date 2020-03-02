#include "../include/CompressedTextureSrc.h"
#include <algorithm>

namespace mineola {
  void CompressedTextureSrc::SetTexture(
    uint32_t layers, uint32_t faces, uint32_t levels,
    uint32_t width, uint32_t height, uint32_t depth,
    std::vector<std::vector<char>> buffers) {
    layers_  = layers;
    faces_ = faces;
    levels_ = levels;
    dimensions_.clear();
    dimensions_.push_back(glm::ivec3(width, height, depth));
    for (uint32_t level = 1; level < levels; ++level) {
      dimensions_.push_back(
        glm::ivec3(
          std::max<int>(1, dimensions_[level - 1].x / 2),
          std::max<int>(1, dimensions_[level - 1].y / 2),
          std::max<int>(1, dimensions_[level - 1].z / 2)));
    }
    buffer_ = std::move(buffers);
  }

  const void *CompressedTextureSrc::Data(uint32_t layer, uint32_t face, uint32_t level) const {
    return buffer_[CalcLevelIdx(layer, face, level)].data();
  }

  uint32_t CompressedTextureSrc::DataSize(uint32_t level) const {
    return (uint32_t)buffer_[CalcLevelIdx(0, 0, level)].size();
  }

  glm::ivec3 CompressedTextureSrc::Dimensions(uint32_t level) const {
    return dimensions_[level];
  }

  int CompressedTextureSrc::Alignment(uint32_t level) const {
    return 1;
  }

  uint32_t CompressedTextureSrc::CalcLevelIdx(
    uint32_t layer, uint32_t face, uint32_t level) const {
    return levels_ * faces_ * layer + levels_ * face + level;
  }
}
