#ifndef MINEOLA_PIXELTYPE_H
#define MINEOLA_PIXELTYPE_H

#include <tuple>
#include <cstdint>

namespace mineola { namespace gl {

uint32_t MapFilterMode(uint32_t wrap_mode);
uint32_t MapWrapMode(uint32_t wrap_mode);

// returns <internal_type, pixel_type, data_type>
std::tuple<uint32_t, uint32_t, uint32_t> MapPixelType(
  uint8_t bpc, uint32_t channel, bool is_signed, bool is_float, bool srgb);

}} //namespace

#endif
