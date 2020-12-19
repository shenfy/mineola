#include "prefix.h"

#ifdef MINEOLA_USE_STBIMAGE

#include <mineola/STBImagePlugin.h>
#include <imgpp/imgpp.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

namespace mineola {

bool STBLoadImageFromFile(const char *fn, bool bottom_first, imgpp::Img &img) {
  if (bottom_first) {
    return false;  // JPG & PNG are top first
  }

  int x = 0, y = 0, n = 0;
  uint8_t *data = stbi_load(fn, &x, &y, &n, 0);
  if (data != nullptr) {
    img.SetSize((uint32_t)x, (uint32_t)y, 1, (uint32_t)n, 8);
    memcpy(img.Data().GetBuffer(), data, x * y * n);
    stbi_image_free(data);
    return true;
  }

  return false;
}

bool STBLoadImageFromMem(const char *buffer, uint32_t length, bool bottom_first, imgpp::Img &img) {
  if (bottom_first) {
    return false;  // JPG & PNG are top first
  }

  int x = 0, y = 0, n = 0;
  uint8_t *data = stbi_load_from_memory((const uint8_t*)buffer, (int)length, &x, &y, &n, 0);
  if (data != nullptr) {
    img.SetSize((uint32_t)x, (uint32_t)y, 1, (uint32_t)n, 8);
    memcpy(img.Data().GetBuffer(), data, x * y * n);
    stbi_image_free(data);
    return true;
  }

  return false;
}

} //end namespace

#endif
