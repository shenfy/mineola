#ifndef IMGPP_SAMPLE_HPP
#define IMGPP_SAMPLE_HPP

#include <cmath>
#include "Imgpp.hpp"
#include "ImgppTypeTraits.hpp"

namespace mineola {
namespace imgpp {

template<typename T>
T Tex2DNN(const ImgROI &roi, float x, float y) {
  uint32_t ui = (uint32_t)(x + 0.5f);
  uint32_t vi = (uint32_t)(y + 0.5f);

  return roi.At<T>(ui, vi);
}

template<typename T>
T Tex2DBilinear(const ImgROI &roi, float x, float y) {
  uint32_t u0 = std::max((uint32_t)floor(x), 0u);
  uint32_t v0 = std::max((uint32_t)floor(y), 0u);
  uint32_t u1 = std::min(u0 + 1, roi.Width() - 1);
  uint32_t v1 = std::min(v0 + 1, roi.Height() - 1);
  float u = x - u0;
  float v = y - v0;

  auto val_u0 = roi.At<T>(u0, v0);
  auto val_u1 = roi.At<T>(u1, v0);
  auto val_u2 = roi.At<T>(u0, v1);
  auto val_u3 = roi.At<T>(u1, v1);
  auto val_uv0 = static_cast<typename Interpolatable<T>::type>(val_u0) * (1.0f - u)
    + static_cast<typename Interpolatable<T>::type>(val_u1) * u;
  auto val_uv1 = static_cast<typename Interpolatable<T>::type>(val_u2) * (1.0f - u)
    + static_cast<typename Interpolatable<T>::type>(val_u3) * u;
  return static_cast<T>(val_uv0 * (1.0f - v) + val_uv1 * v);
}

}}  //namespace imgpp

#endif //IMGPP_SAMPLE_HPP
