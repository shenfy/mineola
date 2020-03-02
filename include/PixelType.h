#ifndef MINEOLA_PIXELTYPE_H
#define MINEOLA_PIXELTYPE_H

namespace mineola { namespace pixel_type {

inline void Map2GL(
  uint8_t bpc, uint32_t channel, bool is_signed, bool is_float, bool srgb,  //inputs
  uint32_t &internal_type, uint32_t &pixel_type, uint32_t &data_type) {   //outputs
  if (1 == channel) {
    pixel_type = GL_RED;
    if (8 == bpc) {
      internal_type = is_signed ? GL_R8_SNORM : GL_R8;
    } else if (16 == bpc) {
      if (is_float) internal_type = GL_R16F;
    } else if (32 == bpc) {
      if (is_float) internal_type = GL_R32F;
      else {
        pixel_type = GL_RED_INTEGER;
        internal_type = is_signed ? GL_R32I : GL_R32UI;  // no normalization
      }
    }

  } else if (2 == channel) {
    pixel_type = GL_RG;
    if (8 == bpc) {
      internal_type = is_signed ? GL_RG8_SNORM : GL_RG8;
    } else if (16 == bpc) {
      if (is_float) internal_type = GL_RG16F;
    } else if (32 == bpc) {
      if (is_float) internal_type = GL_RG32F;
      else {
        pixel_type = GL_RG_INTEGER;
        internal_type = is_signed ? GL_RG32I : GL_RG32UI;  // no normalization
      }
    }
  } else if (3 == channel) {
    pixel_type = GL_RGB;
    if (8 == bpc) {
      if (srgb) {
        internal_type = GL_SRGB8;
      } else {
        internal_type = is_signed ? GL_RGB8_SNORM : GL_RGB8;
      }
    } else if (16 == bpc) {
      if (is_float) internal_type = GL_RGB16F;
    } else if (32 == bpc) {
      if (is_float) internal_type = GL_RGB32F;
      else {
        pixel_type = GL_RGB_INTEGER;
        internal_type = is_signed ? GL_RGB32I : GL_RGB32UI;  // no normalization
      }
    }
  } else if (4 == channel) {
    pixel_type = GL_RGBA;
    if (8 == bpc) {
      if (srgb) {
        internal_type = GL_SRGB8_ALPHA8;
      } else {
        internal_type = is_signed ? GL_RGBA8_SNORM : GL_RGBA8;
      }
    } else if (16 == bpc) {
      if (is_float) internal_type = GL_RGBA16F;
    } else if (32 == bpc) {
      if (is_float) internal_type = GL_RGBA32F;
      else {
        pixel_type = GL_RGBA_INTEGER;
        internal_type = is_signed ? GL_RGBA32I : GL_RGBA32UI;
      }
    }
  }

  if (is_float) data_type = GL_FLOAT;  // (TODO:) support 16bit half float
  else if (8 == bpc)
    data_type = is_signed ? GL_BYTE : GL_UNSIGNED_BYTE;
  else if (16 == bpc)
    data_type = is_signed ? GL_SHORT : GL_UNSIGNED_SHORT;
  else if (32 == bpc)
    data_type = is_signed ? GL_INT : GL_UNSIGNED_INT;

}

}} //namespace

#endif
