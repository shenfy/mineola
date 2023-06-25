#ifndef IMGPP_OPENCVBINDING_HPP
#define IMGPP_OPENCVBINDING_HPP

#include "Imgpp.hpp"
#include <opencv2/core.hpp>

namespace mineola {
namespace imgpp {

  inline int Desc2CVDepth(uint32_t bpc, bool is_signed, bool is_float) {
    switch (bpc) {
      case 8:
        if (is_signed) {
          return CV_8S;
        } else {
          return CV_8U;
        }
        break;
      case 16:
        if (is_float) {
#if CV_VERSION_MAJOR >= 4
          return CV_16F;
#else
          return -1;
#endif
        } else if (is_signed) {
          return CV_16S;
        } else {
          return CV_16U;
        }
        break;
      case 32:
        if (is_float) {
          return CV_32F;
        } else if (is_signed) {
          return CV_32S;
        } else {
          return -1;
        }
        break;
      case 64:
        if (is_float) {
          return CV_64F;
        } else {
          return -1;
        }
        break;
      default:
        return -1;
    }
  }

  // returns tuple<bpc, is_signed, is_float>
  inline std::tuple<uint32_t, bool, bool> CVDepth2Desc(int depth) {
    switch (depth) {
      case CV_8U:
        return std::make_tuple(8, false, false);
      case CV_8S:
        return std::make_tuple(8, true, false);
      case CV_16U:
        return std::make_tuple(16, false, false);
      case CV_16S:
        return std::make_tuple(16, true, false);
#if CV_VERSION_MAJOR >= 4
      case CV_16F:
        return std::make_tuple(16, true, true);
#endif
      case CV_32S:
        return std::make_tuple(32, true, false);
      case CV_32F:
        return std::make_tuple(32, true, true);
      case CV_64F:
        return std::make_tuple(64, true, true);
      default:
        return std::make_tuple(0, false, false);
    }
  }

  inline cv::Mat RefCVMat(ImgROI &roi) {
    int w = (int)roi.Width();
    int h = (int)roi.Height();
    int depth = Desc2CVDepth(roi.BPC(), roi.IsSigned(), roi.IsFloat());
    if (depth < 0) {
      throw std::invalid_argument("No matching cv::Mat format!");
    }
    int type = CV_MAKETYPE(depth, roi.Channel());
    return cv::Mat(h, w, type, const_cast<uint8_t*>(roi.GetData()), roi.Pitch());
  }

  inline ImgROI RefROI(cv::Mat &mat) {
    uint32_t bpc = 0;
    bool is_signed = false;
    bool is_float = false;
    std::tie(bpc, is_signed, is_float) = CVDepth2Desc(mat.depth());
    if (bpc == 0) {
      throw std::invalid_argument("Invalid cv::Mat bit depth!");
    }
    uint32_t pitch = (uint32_t)mat.step[0];
    return ImgROI(mat.data,
      (uint32_t)mat.cols, (uint32_t)mat.rows, mat.channels(),
      bpc, pitch, is_float, is_signed);
  }

}} //namespace imgpp

#endif //IMGPP_OPENCVBINDING_HPP
