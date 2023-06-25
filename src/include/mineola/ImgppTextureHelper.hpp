#ifndef IMGPP_TEXTUREHELPER_HPP
#define IMGPP_TEXTUREHELPER_HPP

/*! \file texturehelper.hpp */

#include <tuple>
#include <mineola/ImgppTextureDesc.hpp>
#include <mineola/ImgppBlockImg.hpp>

namespace mineola { namespace imgpp {

/*! \fn const auto &GetPixelDesc(TextureFormat format)
    \Get pixel description of specified texture format.
    \param format Specified texture format, make sure it is not a compressed format.
    \return std::tuple<uint32_t, uint32_t, uint8_t, bool, bool>, which contains channel, bpc, packed_bytes, is_signed, is_float.
*/
inline const auto &GetPixelDesc(TextureFormat format) {
  using PixelDesc = std::tuple<uint32_t, uint32_t, uint8_t, bool, bool>;
  static const PixelDesc descs[] {
    { 2, 4,  1, false, false },
    { 3, 4,  2, false, false },
    { 1, 16, 2, false, false },
    { 1, 16, 2, false, false },

    { 1, 8, 1, false, false },
    { 1, 8, 1, true,  false },
    { 1, 8, 1, false, false },
    { 1, 8, 1, true,  false },
    { 1, 8, 1, false, false },

    { 2, 8, 1, false, false },
    { 2, 8, 1, true,  false },
    { 2, 8, 1, false, false },
    { 2, 8, 1, true,  false },
    { 2, 8, 1, false, false },

    { 3, 8, 1, false, false },
    { 3, 8, 1, true,  false },
    { 3, 8, 1, false, false },
    { 3, 8, 1, true,  false },
    { 3, 8, 1, false, false },

    { 4, 8, 1, false, false },
    { 4, 8, 1, true,  false },
    { 4, 8, 1, false, false },
    { 4, 8, 1, true,  false },
    { 4, 8, 1, false, false },

    { 1, 32, 4, false, false },
    { 1, 32, 4, true,  false },
    { 1, 32, 4, false, false },
    { 1, 32, 4, true,  false },

    { 1, 16, 2, false, false },
    { 1, 16, 2, true,  false },
    { 1, 16, 2, false, false },
    { 1, 16, 2, true,  false },
    { 1, 16, 2, true,  true },

    { 2, 16, 2, false, false },
    { 2, 16, 2, true, false },
    { 2, 16, 2, false, false },
    { 2, 16, 2, true, false },
    { 2, 16, 2, true, true },

    { 3, 16, 2, false, false },
    { 3, 16, 2, true,  false },
    { 3, 16, 2, false, false },
    { 3, 16, 2, true,  false },
    { 3, 16, 2, true,  true },

    { 4, 16, 2, false, false },
    { 4, 16, 2, true, false },
    { 4, 16, 2, false, false },
    { 4, 16, 2, true, false },
    { 4, 16, 2, true, true },

    { 1, 32, 4, false, false },
    { 1, 32, 4, true,  false },
    { 1, 32, 4, true,  true },

    { 2, 32, 4, false, false },
    { 2, 32, 4, true,  false },
    { 2, 32, 4, true,  true },

    { 3, 32, 4, false, false },
    { 3, 32, 4, true,  false },
    { 3, 32, 4, true,  true },

    { 4, 32, 4, false, false },
    { 4, 32, 4, true,  false },
    { 4, 32, 4, true,  true },

    { 1, 64, 8, false, false },
    { 1, 64, 8, true,  false },
    { 1, 64, 8, true,  true },

    { 2, 64, 8, false, false },
    { 2, 64, 8, true,  false },
    { 2, 64, 8, true,  true },

    { 3, 64, 8, false, false },
    { 3, 64, 8, true,  false },
    { 3, 64, 8, true,  true },

    { 4, 64, 8, false, false },
    { 4, 64, 8, true,  false },
    { 4, 64, 8, true,  true }
  };
  return descs[(int)(format - FORMAT_FIRST)];
}

/*! \fn const BlockSize &GetBlockSize(TextureFormat format)
    \Get block size of specified compressed texture format.
    \param format Specified texture format, make sure it is a compressed format.
    \return BlockSize, which contains block width, block height, block bytes.
*/
inline const BlockSize &GetBlockSize(TextureFormat format) {
  static const BlockSize block_sizes[] = {
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4, 16 },

    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4, 16 },
    { 4, 4, 16 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4, 16 },
    { 4, 4, 16 },

    { 4, 4, 16 },
    { 4, 4, 16 },
    { 5, 4, 16 },
    { 5, 4, 16 },
    { 5, 5, 16 },
    { 5, 5, 16 },
    { 6, 5, 16 },
    { 6, 5, 16 },
    { 6, 6, 16 },
    { 6, 6, 16 },
    { 8, 5, 16 },
    { 8, 5, 16 },
    { 8, 6, 16 },
    { 8, 6, 16 },
    { 8, 8, 16 },
    { 8, 8, 16 },
    { 10, 5, 16 },
    { 10, 5, 16 },
    { 10, 6, 16 },
    { 10, 6, 16 },
    { 10, 8, 16 },
    { 10, 8, 16 },
    { 10, 10, 16 },
    { 10, 10, 16 },
    { 12, 10, 16 },
    { 12, 10, 16 },
    { 12, 12, 16 },
    { 12, 12, 16 },

    { 8, 8, 32 },
    { 8, 8, 32 },
    { 16, 8, 32 },
    { 16, 8, 32 },
    { 8, 8, 32 },
    { 8, 8, 32 },
    { 16, 8, 32 },
    { 16, 8, 32 },
    { 4, 4,  8 },
    { 4, 4,  8 },
    { 8, 4,  8 },
    { 8, 4,  8 },

    { 4, 4,  8 },
    { 4, 4,  8 },
    { 4, 4, 16 },
    { 4, 4, 16 }
  };
  return block_sizes[(int)(format - FORMAT_BLOCK_COMPRESSION_START)];
}
}} // namespaces
#endif
