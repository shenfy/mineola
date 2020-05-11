#ifndef MINEOLA_KTXIMAGELOADER_H
#define MINEOLA_KTXIMAGELOADER_H

#include <cstdint>
#include <memory>

namespace mineola {
class ImgppTextureSrc;

// KTX 1.1 specification
// https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
struct KTXHeader {
  uint8_t identifier[12];
  uint32_t endianness;
  uint32_t gl_type;
  uint32_t gl_type_size;
  uint32_t gl_format;
  uint32_t gl_internal_format;
  uint32_t gl_base_internal_format;
  uint32_t pixel_width;
  uint32_t pixel_height;
  uint32_t pixel_depth;
  uint32_t number_of_array_elements;
  uint32_t number_of_faces;
  uint32_t number_of_mipmap_levels;
  uint32_t bytes_of_key_value_data;
};


std::shared_ptr<ImgppTextureSrc> LoadKTXFromFile(const char *fn);
std::shared_ptr<ImgppTextureSrc> LoadKTXFromMem(const char *buffer, uint32_t length);
}

#endif // MINEOLA_KTXIMAGELOADER_H
