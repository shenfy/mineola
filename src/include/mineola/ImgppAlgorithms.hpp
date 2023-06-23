#ifndef IMGPP_ALGORITHMS_HPP
#define IMGPP_ALGORITHMS_HPP

#include <type_traits>
#include <array>
#include "Imgpp.hpp"

namespace mineola { namespace imgpp {

template<int First, int Last, typename TCallback>
inline void static_for(const TCallback &callback) {
  if constexpr(First < Last) {
    callback(std::integral_constant<int, First>{});
    static_for<First + 1, Last>(callback);
  }
}

/**
 * @brief Transform multiple images simultaneously.
 * @details Iterate through multiple images together and perform pixel/channel-wise actions.
 *
 * @param imgs an std::tuple or std::array of ImgROIs
 * @param callback callback function receiving (x, y, z, c, ptrs) as argument.
 */
template<typename TImgs, typename TCallback>
void ZipTransform(TImgs &&imgs, TCallback callback) {
  constexpr size_t num_imgs = std::tuple_size<typename std::remove_reference<TImgs>::type>::value;
  std::array<uint8_t*, num_imgs> ptrs;
  std::array<uint8_t, num_imgs> steps;

  uint32_t w = std::get<0>(imgs).Width();
  uint32_t h = std::get<0>(imgs).Height();
  uint32_t d = std::get<0>(imgs).Depth();
  uint32_t ch = std::get<0>(imgs).Channel();

  static_for<0, num_imgs>([&steps, &imgs](auto idx) {
    steps[idx] = std::get<idx.value>(imgs).BPC() >> 3;
  });

  for (uint32_t z = 0; z < d; z++) {
    for (uint32_t y = 0; y < h; y++) {

      static_for<0, num_imgs>([&ptrs, &imgs, y, z](auto idx) {
        ptrs[idx] = (uint8_t*)std::get<idx.value>(imgs).PtrAt(0, y, z, 0);
      });

      for (uint32_t x = 0; x < w; x++) {
        for (uint32_t c = 0; c < ch; c++) {

          callback(x, y, z, c, ptrs);

          static_for<0, num_imgs>([&ptrs, &steps](auto idx) {
            ptrs[idx] += steps[idx];
          });
        }
      }
    }
  }
}

/**
 * @brief Transform multiple images pixel by pixel simultaneously.
 * @details Iterate through multiple images together and perform pixel/channel-wise actions.
 *
 * @param imgs an std::tuple or std::array of ImgROIs
 * @param callback callback function receiving (x, y, z, ptrs) as argument.
 */
template<typename TImgs, typename TCallback>
void ZipTransformPixel(TImgs &&imgs, TCallback callback) {
  constexpr size_t num_imgs = std::tuple_size<typename std::remove_reference<TImgs>::type>::value;
  std::array<uint8_t*, num_imgs> ptrs;
  std::array<uint32_t, num_imgs> steps;

  uint32_t w = std::get<0>(imgs).Width();
  uint32_t h = std::get<0>(imgs).Height();
  uint32_t d = std::get<0>(imgs).Depth();

  static_for<0, num_imgs>([&steps, &imgs](auto idx) {
    auto num_ch = std::get<idx.value>(imgs).Channel();
    steps[idx] = (std::get<idx.value>(imgs).BPC() >> 3) * num_ch;
  });

  for (uint32_t z = 0; z < d; z++) {
    for (uint32_t y = 0; y < h; y++) {

      static_for<0, num_imgs>([&ptrs, &imgs, y, z](auto idx) {
        ptrs[idx] = (uint8_t*)std::get<idx.value>(imgs).PtrAt(0, y, z, 0);
      });

      for (uint32_t x = 0; x < w; x++) {

        callback(x, y, z, ptrs);

        static_for<0, num_imgs>([&ptrs, &steps](auto idx) {
          ptrs[idx] += steps[idx];
        });
      }
    }
  }
}
/**
 * @brief Fill ROI with value.
 * @details Fill every channel of every pixel of the same value.
 *
 * @param roi ROI to fill.
 * @param val value to fill.
 */
template<typename T>
void Fill(ImgROI &roi, T val) {
  uint32_t w = roi.Width();
  uint32_t h = roi.Height();
  uint32_t d = roi.Depth();
  uint32_t ch = roi.Channel();

  uint8_t *ptr = 0;
  uint8_t step = roi.BPC() >> 3;

  for (uint32_t z = 0; z < d; z++) {
    for (uint32_t y = 0; y < h; y++) {
      ptr = (uint8_t*)roi.PtrAt(0, y, z, 0);

      for (uint32_t x = 0; x < w * ch; x++) {
        *(T*)ptr = val;
        ptr += step;
      }
    }
  }
}

} // namespace imgpp
} // namespace mineola
#endif
