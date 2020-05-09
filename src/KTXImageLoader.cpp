#include "../include/KTXImageLoader.h"
#include <fstream>
#include <imgpp/bcimgpp.hpp>
#include <imgpp/bchelper.hpp>
#include "../include/ImgppTextureSrc.h"

namespace mineola {

bool LoadKTXFromFile(const char *fn, std::shared_ptr<ImgppTextureSrc> &tex_src) {
  std::ifstream in(fn, std::ios::binary);
  if(!in.good()) {
    return false;
  }
  KTXHeader ktx_header;
  in.read((char *)(&ktx_header), sizeof(KTXHeader));
  in.seekg(ktx_header.bytes_of_key_value_data, std::ios::cur);
  imgpp::BCFormat format = imgpp::KHRToBCFormat(ktx_header.gl_internal_format);
  if (format == imgpp::UNKNOWN) {
    // Not Support uncompressed texture in ktx
    return false;
  }
  tex_src = std::make_shared<ImgppTextureSrc>(
    ktx_header.number_of_faces, ktx_header.number_of_mipmap_levels,
    ktx_header.number_of_array_elements != 0, true);

  for (int level = 0; level < ktx_header.number_of_mipmap_levels; ++level) {
    uint32_t data_size;
    in.read((char*)(&data_size), sizeof(uint32_t));
    if (data_size % 4 != 0) {
      // BC texture data bytes size should multiple of 4;
      tex_src.reset();
      return false;
    }
    uint32_t depth = std::max(ktx_header.number_of_array_elements, ktx_header.number_of_faces);
    imgpp::BCImg img(format, ktx_header.pixel_width, ktx_header.pixel_height, depth);
    in.read((char*)(img.ROI().GetData()), data_size);
    tex_src->AddBuffer(img.Data());
    tex_src->AddBCROI(img.ROI());
  }
  return true;
}

bool LoadKTXFromMem(const char *buffer, uint32_t length, std::shared_ptr<ImgppTextureSrc> &tex_src) {
  KTXHeader const &ktx_header(*reinterpret_cast<KTXHeader const*>(buffer));
  imgpp::BCFormat format = imgpp::KHRToBCFormat(ktx_header.gl_internal_format);
  if (format != imgpp::UNKNOWN) {
    // Not Support uncompressed texture in ktx
    return false;
  }
  size_t offset = sizeof(KTXHeader);
  offset += ktx_header.bytes_of_key_value_data;

  tex_src = std::make_shared<ImgppTextureSrc>(
    ktx_header.number_of_faces, ktx_header.number_of_mipmap_levels,
    ktx_header.number_of_array_elements != 0, true);

  for (int level = 0; level < ktx_header.number_of_mipmap_levels; ++level) {
    uint32_t data_size = *(buffer + offset);
    if (data_size % 4 != 0) {
      // BC texture data bytes size should multiple of 4;
      tex_src.reset();
      return false;
    }
    offset += sizeof(uint32_t);
    uint32_t depth = std::max(ktx_header.number_of_array_elements, ktx_header.number_of_faces);
    imgpp::BCImg img(format, ktx_header.pixel_width, ktx_header.pixel_height, depth);
    std::memcpy((char*)img.ROI().GetData(), buffer + offset, data_size);
    tex_src->AddBuffer(img.Data());
    tex_src->AddBCROI(img.ROI());
    offset += data_size;
  }
  return true;
}

}
