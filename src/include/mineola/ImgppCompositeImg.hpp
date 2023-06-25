#ifndef IMGPP_COMPOSITEIMG_HPP
#define IMGPP_COMPOSITEIMG_HPP

/*! \file compositeimg.hpp */

#include <vector>
#include <variant>
#include <algorithm>
#include <mineola/Imgpp.hpp>
#include <mineola/ImgppBlockImg.hpp>
#include <mineola/ImgppTextureDesc.hpp>
#include <mineola/ImgppTextureHelper.hpp>

namespace mineola { namespace imgpp {

//! \brief Wrapper class for multiple levels/layers/faces image data of a texture
class CompositeImg {
public:
  CompositeImg() = default;

  ~CompositeImg() = default;

  //! \brief Get metadata of the composite image including format, target and if mipmap is requested.
  const TextureDesc &TexDesc() const {
    return tex_desc_;
  }

  //! \brief Get metadata of the composite image including format, target and if mipmap is requested.
  TextureDesc &TexDesc() {
    return tex_desc_;
  }

  //! \brief Specifies texture description, dimensions and alignment of a uncompressed texture.
  //! \param desc specify texture format and usage
  //! \param levels number of mipmap levels
  //! \param layers number of images in an image array
  //! \param faces number of faces in a layer (e.g. 6 for cubemap)
  //! \param width width of a single face
  //! \param height height of a single face
  //! \param depth of a single face (1 for 2D textures)
  //! \param alignment byte alignment of each row/scanline
  void SetSize(const TextureDesc &desc, uint32_t levels, uint32_t layers, uint32_t faces,
    uint32_t width, uint32_t height, uint32_t depth, uint8_t alignment) {
    if (desc.format == FORMAT_UNDEFINED) {
      return;
    }
    auto& pixel_desc = GetPixelDesc(desc.format);
    if (alignment % std::get<2>(pixel_desc) != 0) {
      return;
    }

    tex_desc_ = desc;
    uint32_t bpc = std::get<1>(pixel_desc);
    uint32_t c = std::get<0>(pixel_desc);
    levels_ = levels;
    layers_ = layers;
    faces_ = faces;
    alignment_ = alignment;
    std::vector<ImgROI> rois(levels * layers * faces);
    uint32_t pitch = ImgROI::CalcPitch(width, c, bpc, alignment);
    rois[0] = ImgROI(nullptr, width, height, depth, c, bpc,
      pitch, pitch * height, std::get<3>(pixel_desc), std::get<4>(pixel_desc));
    rois_ = std::move(rois);
  }

  //! \brief Specifies texture description, dimensions and alignment of a compressed texture.
  //! \param desc specify texture format and usage
  //! \param levels number of mipmap levels
  //! \param layers number of images in an image array
  //! \param faces number of faces in a layer (e.g. 6 for cubemap)
  //! \param width width of a single face
  //! \param height height of a single face
  //! \param depth of a single face (1 for 2D textures)
  void SetBCSize(TextureDesc desc, uint32_t levels, uint32_t layers, uint32_t faces,
    uint32_t width, uint32_t height, uint32_t depth) {
    if (desc.format != FORMAT_UNDEFINED) {
      tex_desc_ = std::move(desc);
      levels_ = levels;
      layers_ = layers;
      faces_ = faces;
      std::vector<BlockImgROI> rois(levels * layers * faces);
      const BlockSize &block_size = GetBlockSize(tex_desc_.format);
      rois[0] = BlockImgROI(nullptr, block_size, width, height, depth);
      rois_ = std::move(rois);
    }
  }


  //! \brief Create roi for specified dimension.
  //! \param data a data pointer to the source where the ROI refers to
  //! \param level mipmap level
  //! \param layer array index
  //! \param face face index
  void SetData(uint8_t *data, uint32_t level, uint32_t layer, uint32_t face) {
    if (tex_desc_.format == FORMAT_UNDEFINED) {
      return;
    }
    if (IsCompressed()) {
      auto &rois = std::get<std::vector<BlockImgROI>>(rois_);
      uint32_t width = std::max(rois[0].Width() >> level, 1u);
      uint32_t height = std::max(rois[0].Height() >> level, 1u);
      uint32_t depth = std::max(rois[0].Depth() >> level, 1u);
      rois[level * layers_ * faces_ + layer * faces_ + face] =
        BlockImgROI(data, rois[0].BlkSize(), width, height, depth);
    } else {
      auto &rois = std::get<std::vector<ImgROI>>(rois_);
      uint32_t width = std::max(rois[0].Width() >> level, 1u);
      uint32_t height = std::max(rois[0].Height() >> level, 1u);
      uint32_t depth = std::max(rois[0].Depth() >> level, 1u);
      uint32_t pitch = ImgROI::CalcPitch(width, rois[0].Channel(), rois[0].BPC(), alignment_);
      rois[level * layers_ * faces_ + layer * faces_ + face] =
        ImgROI(data, width, height, depth,
          rois[0].Channel(), rois[0].BPC(), pitch, pitch * height,
          rois[0].IsFloat(), rois[0].IsSigned());
    }
  }

  //! \brief Add an image buffer to be held inside the CompositeImg.
  //! \param buffer an ImgBuffer class holding actual image data
  void AddBuffer(imgpp::ImgBuffer buffer) {
    buffers_.push_back(std::move(buffer));
  }

  const std::vector<ImgBuffer> &Buffers() const {
    return buffers_;
  }

  //! \brief Get a const ROI associated to the specific level/layer/face
  const ImgROI &ROI(uint32_t level, uint32_t layer, uint32_t face) const {
    return std::get<std::vector<ImgROI>>(rois_)[level * layers_ * faces_ + layer * faces_ + face];
  }

  //! \brief Get the ROI associated to the specific level/layer/face
  ImgROI &ROI(uint32_t level, uint32_t layer, uint32_t face) {
    return std::get<std::vector<ImgROI>>(rois_)[level * layers_ * faces_ + layer * faces_ + face];
  }

  //! \brief Get a const BlockROI associated to the specific level/layer/face
  const BlockImgROI &BlockROI(uint32_t level, uint32_t layer, uint32_t face) const {
    return std::get<std::vector<BlockImgROI>>(rois_)[level * layers_ * faces_ + layer * faces_ + face];
  }

  //! \brief Get the BlockROI associated to the specific level/layer/face
  BlockImgROI &BlockROI(uint32_t level, uint32_t layer, uint32_t face) {
    return std::get<std::vector<BlockImgROI>>(rois_)[level * layers_ * faces_ + layer * faces_ + face];
  }

  bool IsCompressed() const {
    return IsCompressedFormat(tex_desc_.format);
  }

  uint32_t Levels() const {
    return levels_;
  }

  uint32_t Layers() const {
    return layers_;
  };

  uint32_t Faces() const {
    return faces_;
  }

  uint32_t Alignment() const {
    return alignment_;
  }

private:
  std::vector<ImgBuffer> buffers_; /*!< a vector of ImgBuffers holding img data */
  std::variant<std::vector<ImgROI>, std::vector<BlockImgROI>> rois_; /*!< a vector of either ImgROI or BlockImgROI depends on texture format */
  TextureDesc tex_desc_;
  uint32_t levels_{0}; /*!< mipmap levels */
  uint32_t layers_{0}; /*!< number of textures in texture array, or equals 1 if not a texture array */
  uint32_t faces_{0}; /*!< 6 for cubemap faces, otherwise 1 */
  uint8_t alignment_{1};
};

}}  // namespaces
#endif
