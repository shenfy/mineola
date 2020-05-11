#include "../include/KTXImageLoader.h"
#include <fstream>
#include <imgpp/imgpp_bc.hpp>
#include <imgpp/bchelper.hpp>
#include "../include/ImgppTextureSrc.h"

namespace mineola {

std::shared_ptr<ImgppTextureSrc> LoadKTXFromFile(const char *fn) {
  std::ifstream in(fn, std::ios::binary);
  if(!in.good()) {
    return {};
  }
  KTXHeader ktx_header;
  in.read((char *)(&ktx_header), sizeof(KTXHeader));
  in.seekg(ktx_header.bytes_of_key_value_data, std::ios::cur);
  imgpp::BCFormat format = imgpp::KHRToBCFormat(ktx_header.gl_internal_format);
  if (format == imgpp::UNKNOWN_BC) {
    // Not Support uncompressed texture in ktx
    return {};
  }
  uint32_t layers = ktx_header.number_of_array_elements > 0
    ? ktx_header.number_of_array_elements : 1;
  auto tex_src = std::make_shared<ImgppTextureSrc>(
    ktx_header.number_of_faces, layers, ktx_header.number_of_mipmap_levels);

  for (int level = 0; level < ktx_header.number_of_mipmap_levels; ++level) {
    uint32_t data_size;
    in.read((char*)(&data_size), sizeof(uint32_t));
    if (data_size % 4 != 0) {
      // BC texture data bytes size should multiple of 4;
      return {};
    }
    uint32_t width = ktx_header.pixel_width >> level;
    uint32_t height = ktx_header.pixel_height >> level;
    uint32_t depth = tex_src->Layers() * tex_src->Faces();
    imgpp::BCImg img(format, width, height, depth);
    in.read((char*)(img.ROI().GetData()), data_size);
    tex_src->AddBuffer(img.Data());
    tex_src->AddBCROI(img.ROI());
  }
  return tex_src;
}

std::shared_ptr<ImgppTextureSrc> LoadKTXFromMem(const char *buffer, uint32_t length) {
  KTXHeader const &ktx_header(*reinterpret_cast<KTXHeader const*>(buffer));
  imgpp::BCFormat format = imgpp::KHRToBCFormat(ktx_header.gl_internal_format);
  if (format != imgpp::UNKNOWN_BC) {
    // Not Support uncompressed texture in ktx
    return {};
  }
  size_t offset = sizeof(KTXHeader);
  offset += ktx_header.bytes_of_key_value_data;

  uint32_t layers = ktx_header.number_of_array_elements > 0
    ? ktx_header.number_of_array_elements : 1;
  auto tex_src = std::make_shared<ImgppTextureSrc>(
    ktx_header.number_of_faces, layers, ktx_header.number_of_mipmap_levels);

  for (int level = 0; level < ktx_header.number_of_mipmap_levels; ++level) {
    uint32_t data_size = *(buffer + offset);
    if (data_size % 4 != 0) {
      // BC texture data bytes size should multiple of 4;
      tex_src.reset();
      return {};
    }
    offset += sizeof(uint32_t);
    uint32_t width = ktx_header.pixel_width >> level;
    uint32_t height = ktx_header.pixel_height >> level;
    uint32_t depth = tex_src->Layers() * tex_src->Faces();
    imgpp::BCImg img(format, width, height, depth);
    std::memcpy((char*)img.ROI().GetData(), buffer + offset, data_size);
    tex_src->AddBuffer(img.Data());
    tex_src->AddBCROI(img.ROI());
    offset += data_size;
  }
  return tex_src;
}

}
