#ifndef IMGPP_BLOCKIMG_HPP
#define IMGPP_BLOCKIMG_HPP

/*! \file blockimg.hpp */

#include <mineola/ImgppImgBase.hpp>
#include <mineola/ImgppTextureDesc.hpp>

namespace mineola {
namespace imgpp {

// !BlockSize is the description of single block in BlockImg
struct BlockSize {
  uint32_t block_width {0};
  uint32_t block_height {0};
  uint32_t block_bytes {0};

  bool operator==(const BlockSize &other) const {
    return block_width == other.block_width && block_height == other.block_height
      && block_bytes == other.block_bytes;
  }

  bool operator!=(const BlockSize &other) const {
    return !this->operator==(other);
  }
};

//! BlockImgROI is a view into an ImgBuffer or a plain C-style buffer which contains img data in blocks

//! BlockImgROI doesn't "own" the buffer memory, hence the user must make sure the pointer buffer_ is valid before accesing data.
class BlockImgROI {
public:
  friend class BlockImg;

  BlockImgROI() {}

  BlockImgROI(uint8_t *src, const BlockSize &block_size,
    uint32_t w, uint32_t h, uint32_t depth, uint32_t pitch, uint32_t slice_pitch) {
    if (block_size.block_bytes != 0 && w != 0 && h != 0 && depth != 0) {
      Init(src, block_size, w, h, depth, pitch, slice_pitch);
    }
  }

  BlockImgROI(uint8_t *src, const BlockSize &block_size, uint32_t w, uint32_t h, uint32_t depth) {
    if (block_size.block_bytes != 0 && w != 0 && h != 0 && depth != 0) {
      uint32_t horizontal_block_num = (w + block_size.block_width - 1) / block_size.block_width;
      uint32_t vertical_block_num = (h + block_size.block_height - 1) / block_size.block_height;
      uint32_t pitch = horizontal_block_num * block_size.block_bytes;
      uint32_t slice_pitch = pitch * vertical_block_num;
      Init(src, block_size, w, h, depth, pitch, slice_pitch);
    }
  }

  BlockImgROI(uint8_t *src, const BlockSize &block_size, uint32_t w, uint32_t h) :
    BlockImgROI(src, block_size, w, h, 1) {}

  BlockImgROI(const BlockImgROI &src,
    uint32_t left, uint32_t top, uint32_t front,
    uint32_t right, uint32_t bottom, uint32_t back) {
    if (right < src.width_ && bottom < src.height_ && back < src.depth_
      && right >= left && bottom >= top && back >= front && src.block_size_.block_bytes != 0) {
      // 2D blocks only!!!
      Init((uint8_t*)(src.BlockAt(left / src.block_size_.block_width, top / src.block_size_.block_height, front)),
        src.block_size_, right - left + 1, bottom - top + 1, back - front + 1, src.pitch_, src.slice_pitch_);
    }
  }

  BlockImgROI SubRegion(
    uint32_t left, uint32_t top, uint32_t front,
    uint32_t right, uint32_t bottom, uint32_t back) {
    BlockImgROI new_roi(*this, left, top, front, right, bottom, back);
    return new_roi;
  }

  void *BlockAt(uint32_t block_x, uint32_t block_y) {
    return data_ + block_y * pitch_ + block_x * block_size_.block_bytes;
  }

  const void *BlockAt(uint32_t block_x, uint32_t block_y) const {
    return data_ + block_y * pitch_ + block_x * block_size_.block_bytes;
  }

  void *BlockAt(uint32_t block_x, uint32_t block_y, uint32_t z) {
    return data_ + z * slice_pitch_ + block_y * pitch_ + block_x * block_size_.block_bytes;
  }

  const void *BlockAt(uint32_t block_x, uint32_t block_y, uint32_t z) const {
    return data_ + z * slice_pitch_ + block_y * pitch_ + block_x * block_size_.block_bytes;
  }

  const BlockSize &BlkSize() const {
    return block_size_;
  }

  uint32_t Width() const {
    return width_;
  }

  uint32_t Height() const {
    return height_;
  }

  uint32_t Depth() const {
    return depth_;
  }

  uint32_t Pitch() const {
    return pitch_;
  }

  uint32_t SlicePitch() const {
    return slice_pitch_;
  }

  uint32_t HorizontalBlockNum() const {
    return horizontal_block_num_;
  }

  uint32_t VerticalBlockNum() const {
    return vertical_block_num_;
  }

  const uint32_t *Dimensions() const {
    return dimensions_;
  }

  const uint8_t *GetData() const {
    return data_;
  }

  uint8_t *GetData() {
    return data_;
  }

  static uint32_t CalcPitch(const BlockSize &block_size, uint32_t w) {
    if (block_size.block_bytes == 0) {
      return 0;
    }
    auto horizontal_block_num = (w + block_size.block_width - 1) / block_size.block_width;
    return horizontal_block_num * block_size.block_bytes;
  }

private:
  void Init(uint8_t *src, const BlockSize &block_size,
    uint32_t w, uint32_t h, uint32_t depth,
    uint32_t pitch, uint32_t slice_pitch) {
    data_ = src;
    block_size_ = block_size;
    width_ = w;
    height_ = h;
    depth_ = depth;
    pitch_ = pitch;
    slice_pitch_ = slice_pitch;
    horizontal_block_num_ = (width_ + block_size_.block_width - 1) / block_size_.block_width;
    vertical_block_num_ = (height_ + block_size_.block_height - 1) / block_size_.block_height;
  }

  uint8_t *data_{nullptr}; //!<Data pointer. NOT a smart pointer hence NOT responsible the buffer!
  BlockSize block_size_;

  union {
    struct {
      uint32_t width_;
      uint32_t height_;
      uint32_t depth_;
      uint32_t horizontal_block_num_;
      uint32_t vertical_block_num_;
    };
    uint32_t dimensions_[5] = {0, 0, 0, 0, 0};
  };

  uint32_t pitch_{0};
  uint32_t slice_pitch_{0};
};

inline bool CopyData(BlockImgROI &dst, const BlockImgROI &src) {
  if (src.BlkSize() != dst.BlkSize() || src.Width() > dst.Width() || src.Height() > dst.Height()
    || src.Depth() > dst.Depth()) {
    return false;
  }

  uint32_t src_pitch = BlockImgROI::CalcPitch(src.BlkSize(), src.Width());
  for (uint32_t z = 0; z < src.Depth(); ++z) {
    for (uint32_t y = 0; y < src.VerticalBlockNum(); ++y) {
      memcpy(dst.BlockAt(0, y, z), src.BlockAt(0, y, z), src_pitch);
    }
  }
  return true;
}

//! BlockImg holds a 2D or 3D block image using ImgBuffer and a BlockImgROI
class BlockImg: public ImgBase<BlockImgROI> {
public:
  BlockImg() {}
  ~BlockImg() {}

  BlockImg(const BlockSize &block_size, uint32_t w, uint32_t h, uint32_t depth = 1) {
    SetSize(block_size, w, h, depth);
  }

  void SetSize(const BlockSize &block_size, uint32_t w, uint32_t h, uint32_t depth) {
    entire_img_ = BlockImgROI(nullptr, block_size, w, h, depth);
    if (entire_img_.width_ == 0) {
      return;
    }
    buffer_.SetSize(entire_img_.slice_pitch_ * entire_img_.depth_);
    entire_img_.data_ = buffer_.GetBuffer();
  }

  void SetSizeLike(const BlockImgROI &src_roi) {
    SetSize(src_roi.block_size_, src_roi.width_, src_roi.height_, src_roi.depth_);
  }

  bool CopyFrom(const BlockImgROI &src_roi) {
    SetSizeLike(src_roi);
    return CopyData(entire_img_, src_roi);
  }
};

}}  // namespaces

#endif
