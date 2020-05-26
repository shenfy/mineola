#ifndef MINEOLA_TYPEMAPPING_H
#define MINEOLA_TYPEMAPPING_H
#include <cstdint>
#include "glutility.h"

namespace mineola { namespace type_mapping {
  enum DataType { UNKNOWN = 0, BOOL, BYTE, UBYTE, INT16, UINT16,
    INT32, UINT32, FLOAT16, FLOAT32, FLOAT64 };

  enum BarrierType {
    VERTEX_ATTRIB_ARRAY_BARRIER = 1 << 0,
    ELEMENT_ARRAY_BARRIER = 1 << 1,
    UNIFORM_BARRIER = 1 << 2,
    TEXTURE_FETCH_BARRIER = 1 << 3,
    SHADER_IMAGE_ACCESS_BARRIER = 1 << 4,
    COMMAND_BARRIER = 1 << 5,
    PIXEL_BUFFER_BARRIER = 1 << 6,
    TEXTURE_UPDATE_BARRIER = 1 << 7,
    BUFFER_UPDATE_BARRIER = 1 << 8,
    FRAMEBUFFER_BARRIER = 1 << 9,
    TRANSFORM_FEEDBACK_BARRIER = 1 << 10,
    ATOMIC_COUNTER_BARRIER = 1 << 11,
    SHADER_STORAGE_BARRIER = 1 << 12 };

  //return size in number of bytes
  inline uint32_t SizeOf(uint32_t type) {
    switch (type) {
      case BOOL:
      case BYTE:
      case UBYTE:
        return 1;
      case INT16:
      case UINT16:
      case FLOAT16:
        return 2;
      case INT32:
      case UINT32:
      case FLOAT32:
        return 4;
      case FLOAT64:
        return 8;
      default:
        return 0;
    }
  }

  inline uint32_t SizeOfGLType(uint32_t type) {
    switch (type) {
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
        return 1;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
        return 2;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
      case GL_FLOAT:
        return 4;
      default:
        return 0;
    }
  }

  inline uint32_t NumChannels(uint32_t type) {
    switch(type) {
      case GL_RED:
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_STENCIL:
        return 1;
      case GL_RG:
        return 2;
      case GL_RGB:
        return 3;
      case GL_RGBA:
        return 4;
      default:
        return 0;
    }
  }

  //doesn't give details of compact pixel formats
  inline void MapGLFormat(
    uint32_t internal_format, uint32_t *bpc, uint32_t *num_channels, bool *is_float) {
    *is_float = false;
    switch (internal_format)
    {
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R8I:
    case GL_R8UI:
      *bpc = 8;
      *num_channels = 1;
      break;
    case GL_R16I:
    case GL_R16UI:
      *bpc = 16;
      *num_channels = 1;
      break;
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG8I:
    case GL_RG8UI:
      *bpc = 8;
      *num_channels = 2;
      break;
    case GL_RG16I:
    case GL_RG16UI:
      *bpc = 8;
      *num_channels = 2;
      break;
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB8I:
    case GL_RGB8UI:
      *bpc = 8;
      *num_channels = 3;
      break;
    case GL_RGB16I:
    case GL_RGB16UI:
      *bpc = 16;
      *num_channels = 3;
      break;
    case GL_RGBA4:
      *bpc = 4;
      *num_channels = 4;
      break;
    case GL_RGB5_A1:
      *bpc = 16;
      *num_channels = 1;
      break;
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGBA8I:
    case GL_RGBA8UI:
      *bpc = 8;
      *num_channels = 4;
      break;
    case GL_RGB10_A2:
    case GL_RGB10_A2UI:
      *bpc = 16;
      *num_channels = 2;
      break;
    case GL_RGBA16I:
    case GL_RGBA16UI:
      *bpc = 16;
      *num_channels = 4;
      break;
    case GL_SRGB8:
      *bpc = 8;
      *num_channels = 3;
      break;
    case GL_SRGB8_ALPHA8:
      *bpc = 8;
      *num_channels = 4;
      break;
    case GL_R16F:
      *bpc = 16;
      *num_channels = 1;
      *is_float = true;
      break;
    case GL_RG16F:
      *bpc = 16;
      *num_channels = 2;
      *is_float = true;
      break;
    case GL_RGB16F:
      *bpc = 16;
      *num_channels = 3;
      *is_float = true;
      break;
    case GL_RGBA16F:
      *bpc = 16;
      *num_channels = 4;
      *is_float = true;
      break;
    case GL_R32F:
      *bpc = 32;
      *num_channels = 1;
      *is_float = true;
      break;
    case GL_RG32F:
      *bpc = 32;
      *num_channels = 2;
      *is_float = true;
      break;
    case GL_RGB32F:
      *bpc = 32;
      *num_channels = 3;
      *is_float = true;
      break;
    case GL_RGBA32F:
      *bpc = 32;
      *num_channels = 4;
      *is_float = true;
      break;
    case GL_R11F_G11F_B10F:
      *bpc = 32;
      *num_channels = 1;
      *is_float = true;
      break;
    case GL_RGB9_E5:
      *bpc = 32;
      *num_channels = 1;
      *is_float = true;
      break;
    case GL_R32I:
    case GL_R32UI:
      *bpc = 32;
      *num_channels = 1;
      break;
    case GL_RG32I:
    case GL_RG32UI:
      *bpc = 32;
      *num_channels = 2;
      break;
    case GL_RGB32I:
    case GL_RGB32UI:
      *bpc = 32;
      *num_channels = 3;
      break;
    case GL_RGBA32I:
    case GL_RGBA32UI:
      *bpc = 32;
      *num_channels = 4;
      break;
    default:
      *bpc = 0;
      *num_channels = 0;
    }
  }

  inline void MapGLType(uint32_t gltype, uint32_t *type, uint32_t *size) {
    switch (gltype) {
      case GL_HALF_FLOAT:
        *type = FLOAT16;
        *size = 1;
        break;
      case GL_FLOAT:
        *type = FLOAT32;
        *size = 1;
        break;
      case GL_FLOAT_VEC2:
        *type = FLOAT32;
        *size = 2;
        break;
      case GL_FLOAT_VEC3:
        *type = FLOAT32;
        *size = 3;
        break;
      case GL_FLOAT_VEC4:
        *type = FLOAT32;
        *size = 4;
        break;
      case GL_FLOAT_MAT2:
        *type = FLOAT32;
        *size = 4;
        break;
      case GL_FLOAT_MAT3:
        *type = FLOAT32;
        *size = 9;
        break;
      case GL_FLOAT_MAT4:
        *type = FLOAT32;
        *size = 16;
        break;
      case GL_FLOAT_MAT2x3:
        *type = FLOAT32;
        *size = 6;
        break;
      case GL_FLOAT_MAT2x4:
        *type = FLOAT32;
        *size = 8;
        break;
      case GL_FLOAT_MAT3x2:
        *type = FLOAT32;
        *size = 6;
        break;
      case GL_FLOAT_MAT3x4:
        *type = FLOAT32;
        *size = 12;
        break;
      case GL_FLOAT_MAT4x2:
        *type = FLOAT32;
        *size = 8;
        break;
      case GL_FLOAT_MAT4x3:
        *type = FLOAT32;
        *size = 12;
        break;
      case GL_INT:
        *type = INT32;
        *size = 1;
        break;
      case GL_INT_VEC2:
        *type = INT32;
        *size = 2;
        break;
      case GL_INT_VEC3:
        *type = INT32;
        *size = 3;
        break;
      case GL_INT_VEC4:
        *type = INT32;
        *size = 4;
        break;
      case GL_UNSIGNED_INT:
        *type = UINT32;
        *size = 1;
        break;
      case GL_UNSIGNED_INT_VEC2:
        *type = UINT32;
        *size = 2;
        break;
      case GL_UNSIGNED_INT_VEC3:
        *type = UINT32;
        *size = 3;
        break;
      case GL_UNSIGNED_INT_VEC4:
        *type = UINT32;
        *size = 4;
        break;
      default:
        *type = UNKNOWN;
        *size = 0;
        break;
    }
  }

  inline uint32_t Map2GLType(uint32_t type) {
    switch (type) {
      case BOOL:
        return GL_BOOL;
      case BYTE:
        return GL_BYTE;
      case UBYTE:
        return GL_UNSIGNED_BYTE;
      case INT16:
        return GL_SHORT;
      case UINT16:
        return GL_UNSIGNED_SHORT;
      case INT32:
        return GL_INT;
      case UINT32:
        return GL_UNSIGNED_INT;
      case FLOAT16:
        return GL_HALF_FLOAT;
      case FLOAT32:
        return GL_FLOAT;
      default:
        return GL_FLOAT;
    }
  }

  inline uint32_t Map2GLType(uint8_t bpc, bool is_float, bool is_signed) {
    uint8_t Bpc = bpc >> 3; //bits to bytes
    uint32_t type = GL_UNSIGNED_BYTE;
    if (Bpc == 1) {
      type = is_signed ? GL_BYTE : GL_UNSIGNED_BYTE;
    } else if (Bpc == 2) {
      if (!is_float)
        type = is_signed ? GL_SHORT : GL_UNSIGNED_SHORT;
      else
        type = GL_HALF_FLOAT;
    } else if (Bpc == 4) {
      if (!is_float)
        type = is_signed ? GL_INT : GL_UNSIGNED_INT;
      else
        type = GL_FLOAT;
    }

    return type;
  }

  inline bool Map2GLDepthFormat(
    uint32_t bits, bool stencil, uint32_t &internal_format, uint32_t &data_type) {
    if (bits == 16) {
      internal_format = GL_DEPTH_COMPONENT16;
      data_type = GL_UNSIGNED_SHORT;
      return true;
    } else if (bits == 24) {
      if (stencil) { internal_format = GL_DEPTH24_STENCIL8; data_type = GL_UNSIGNED_INT_24_8; }
      else { internal_format = GL_DEPTH_COMPONENT24; data_type = GL_UNSIGNED_INT; }
      return true;
    } else if (bits == 32) {
      if (stencil) { internal_format = GL_DEPTH32F_STENCIL8; data_type = GL_FLOAT; }
      else { internal_format = GL_DEPTH_COMPONENT32F; data_type = GL_FLOAT; }
      return true;
    }
    return false;
  }

  inline void MapGLDepthFormat(uint32_t internal_format, uint32_t &bits, bool &stencil) {
    switch (internal_format) {
    case GL_DEPTH_COMPONENT16:
      bits = 16;
      stencil = false;
      break;
    case GL_DEPTH_COMPONENT24:
      bits = 24;
      stencil = false;
      break;
    case GL_DEPTH24_STENCIL8:
      bits = 24;
      stencil = true;
      break;
    case GL_DEPTH_COMPONENT32F:
      bits = 32;
      stencil = false;
      break;
    case GL_DEPTH32F_STENCIL8:
      bits = 32;
      stencil = true;
      break;
    default:
      bits = 0;
      stencil = false;
    }
  }

  inline uint32_t Usage2GL(uint32_t frequency, uint32_t direction) {
    static const uint32_t c_gl[3][3] = {{GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY},
                       {GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY},
                       {GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY}};
    return c_gl[frequency][direction];
  }

  inline uint32_t Access2GL(uint32_t access) {
    static const uint32_t access_gl[3] =
      {GL_MAP_READ_BIT, GL_MAP_WRITE_BIT, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT};
    return access_gl[access];
  }


}} //end namespaces


#endif
