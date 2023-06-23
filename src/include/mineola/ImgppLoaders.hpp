#ifndef IMGPP_LOADERS_HPP
#define IMGPP_LOADERS_HPP

#include <string>
#include <unordered_map>
/*! \file loaders.hpp */

namespace mineola {
namespace imgpp {

  class Img;
  class ImgROI;
  class CompositeImg;
  //! \brief Load netbpm PPM format images.
  //!
  //! Although http://netpbm.sourceforge.net/doc/pgm.html and http://netpbm.sourceforge.net/doc/ppm.html
  //! says "The most significant byte is first.", our loader does not respect that and reads whatever
  //! bytes into memory as they are for performance concern.
  //! \param fn PPM file full path
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPPM(const char *fn, Img &img, bool bottom_first);

  //! \brief Load netbpm PPM format images.
  //!
  //! \param src input buffer containing the PPM data (including the headers)
  //! \param length length of the input buffer
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPPM(const char *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Save an ROI to netbmp PPM file.
  //! Save .ppm file to disk.
  //! \param fn PPM file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param bottom_first whether the content in the roi is bottom first
  bool WritePPM(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Load MS Windows Bitmap from file.
  //! Load .bmp file from disk file.
  //! \param fn BMP file full path
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadBMP(const char *fn, Img &img, bool bottom_first);

  //! \brief Load MS Windows Bitmap.
  //! Parse .bmp image from memory.
  //! \param src input buffer containing the bitmap data (including the headers)
  //! \param length length of the input buffer
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadBMP(const char *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Save an ROI to MS Windows Bitmap file.
  //! Save .bmp file to disk.
  //! \param fn BMP file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param bottom_first whether the content in the roi is bottom first
  bool WriteBMP(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Load netbpm pfm from file.
  //! PFM pixel layout is bottom-to-top.
  //! \param fn PFM file full path
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPFM(const char *fn, Img &img, bool bottom_first);

  //! \brief Parse netbpm pfm from memory buffer.
  //!
  //! PFM pixel layout is bottom-to-top.
  //! \param src input buffer containing the PPM data (including the headers)
  //! \param length length of the input buffer
  //! \param img output imgpp::Img object filled with load data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadPFM(const char *src, uint32_t length, Img &img, bool bottom_first);

  //! \brief Save an ROI to netbmp PFM file.
  //!
  //! Save .pfm file to disk.
  //! \param fn PFM file full path
  //! \param roi input imgpp::ImgROI to dump to file
  //! \param bottom_first whether the content in the roi is bottom first
  bool WritePFM(const char *fn, const ImgROI &roi, bool bottom_first);

  //! \brief Deserialize Img from file.
  //!
  //! \param fn bson file full path
  //! \param img output imgpp::Img object filled with load data
  bool LoadBSON(const char *fn, Img &img);

  //! \brief Deserialize Img from memory buffer.
  //!
  //! \param buffer bson data buffer
  //! \param length bson data buffer size
  //! \param img output imgpp::Img object filled with load data
  bool LoadBSON(const char *buffer, uint32_t length, Img &img);

  //! \brief Serialize an image to a bson data buffer.
  //! \param img imgpp::Img to serialize
  //! \param bson output char buffer containing the serialized binary data
  bool WriteBSON(const Img &img, std::string &bson);

  //! \brief Serialize an image to a bson data file.
  //!
  //! \param fn .bson file full path
  //! \param img imgpp::Img to serialize
  bool WriteBSON(const char *fn, const Img &img);

  //! \brief Serialize an image ROI to a bson data buffer.
  //!
  //! \param roi imgpp::ImgROI to serialize
  //! \param bson output char buffer containing the serialized binary data
  bool WriteBSON(const ImgROI &roi, std::string &bson);

  //! \brief Serialize an image ROI to a bson file.
  //!
  //! \param fn .bson file full path
  //! \param roi imgpp::ImgROI to serialize
  bool WriteBSON(const char *fn, const ImgROI &roi);

  //! \brief Load Khronos KTX1 format images.
  //  Although https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
  //! \param src input buffer containing the ktx data (including the headers)
  //! \param length length of the input buffer
  //! \param CompositeImg output imgpp::CompositeImg object filled with load data
  //! \param custom_data output std::unordered_map<std::string, string> object filled with kv data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadKTX(const char *src, size_t length, CompositeImg &img,
    std::unordered_map<std::string, std::string> &custom_data, bool bottom_first);

  //! \brief Load Khronos KTX1 format images.
  //! \param fn ktx file full path
  //! \param CompositeImg output imgpp::CompositeImg object filled with load data
  //! \param custom_data output std::unordered_map<std::string, string> object filled with kv data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool LoadKTX(const char *fn, CompositeImg &img,
    std::unordered_map<std::string, std::string> &custom_data, bool bottom_first);

  //! \brief Save Khronos KTX1 format images to .ktx file.
  //! \param fn ktx file full path
  //! \param CompositeImg input imgpp::CompositeImg object filled with load data
  //! \param custom_data input std::unordered_map<std::string, string> object filled with kv data
  //! \param bottom_first whether the loaded image data in memory is bottom first
  bool WriteKTX(const char *fn, const CompositeImg &img,
    const std::unordered_map<std::string, std::string> &custom_data, bool bottom_first);
}}

#endif
