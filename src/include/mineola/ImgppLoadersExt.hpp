#ifndef IMGPP_LOADERSEXT_HPP
#define IMGPP_LOADERSEXT_HPP

/*! \file loadersext.hpp */

#include <memory>

namespace mineola {
namespace imgpp {

  class Img;
  class ImgROI;

  //! \brief Load image from file containers of supported formats.
  //!
  //! The format is automatically determined by the file's extension name.
  //! \param fn full path to the file
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool Load(const char *fn, Img &img, bool bottom_first);

  //! \brief Load image of various formats from memory buffer.
  //!
  //! The format is automatically determined by peeking the magic number at the first a few bytes.
  //! \param src input buffer
  //! \param length input buffer size
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool Load(const char *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Write an ROI to file.
  //!
  //! The format is determined by the file's extension name.
  //! \param fn full path to the file
  //! \param roi imgpp::ImgROI to dump
  //! \param bottom_first whether the image data in roi is bottom first
  bool Write(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Load PNG format images.
  //!
  //! \param fn PNG file full path
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPNG(const char *fn, Img &img, bool bottom_first);

  //! \brief Load PNG format image from memory.
  //!
  //! Parse .png image from memory.
  //! \param src input buffer containing the image data (including the headers)
  //! \param length length of the input buffer
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPNG(void *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Save an ROI to PNG file.
  //!
  //! Save .png file to disk.
  //! \param fn PNG file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param bottom_first whether the content in the roi is bottom first
  bool WritePNG(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Compress an ROI to a PNG memory buffer.
  //!
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param dst output buffer containing the compressed
  //! \param length output buffer length (pre-allocated by caller)
  size_t CompressPNG(const ImgROI& roi, void *dst, size_t length);

  //! \brief Load JPEG format images.
  //!
  //! \param fn JPEG file full path
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadJPEG(const char *fn, Img &img, bool bottom_first);

  //! \brief Parse JPEG format image from memory.
  //!
  //! \param src input buffer containing the image data (including the headers)
  //! \param length length of the input buffer
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadJPEG(const void *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Save an ROI to JPEG file.
  //!
  //! \param fn JPEG file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param bottom_first whether the content in the roi is bottom first
  bool WriteJPEG(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Save an ROI to JPEG file at specific quality level.
  //!
  //! \param fn JPEG file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param quality JPEG compression quality level (0-100, suggested value: 75)
  //! \param bottom_first whether the content in the roi is bottom first
  bool WriteJPEGAtQuality(const char *fn, const ImgROI &roi, int quality, bool bottom_first);

  //! \brief Compress an ROI to a JPEG memory buffer.
  //!
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param dst output buffer containing the compressed
  //! \param length output buffer length (pre-allocated by caller)
  uint32_t CompressJPEG(const ImgROI &roi, void *dst, uint32_t length);

}}

#endif