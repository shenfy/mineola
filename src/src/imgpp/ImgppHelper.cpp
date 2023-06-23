#include <mineola/Imgpp.hpp>
#include <mineola/ImgppLoaders.hpp>
#include <mineola/ImgppLoadersExt.hpp>
#include <string>
#include <cstring>
#include <algorithm>

namespace {

bool IsDDSFormat(const char *buffer) {
  const uint32_t *value = (const uint32_t *)buffer;
  return (*value == 0x20534444);
}

bool IsKTXFormat(const char *buffer) {
  auto ub = reinterpret_cast<const uint8_t *>(buffer);
  if (ub[0] == 0xAB && ub[1] == 0x4B && ub[2] == 0x54 && ub[3] == 0x58 &&
    ub[4] == 0x20 && ub[5] == 0x31 && ub[6] == 0x31 && ub[7] == 0xBB &&
    ub[8] == 0x0D && ub[9] == 0x0A && ub[10] == 0x1A && ub[11] == 0x0A) {
    return true;
  }
  return false;
}

bool IsPNGFormat(const char *buffer) {
  auto ub = reinterpret_cast<const uint8_t *>(buffer);
  if (ub[0] == 0x89 && ub[1] == 0x50 && ub[2] == 0x4E && ub[3] == 0x47 &&
    ub[4] == 0x0D && ub[5] == 0x0A && ub[6] == 0x1A && ub[7] == 0x0A) {
    return true;
  }
  return false;
}

bool IsJPEGFormat(const char *buffer) {
  auto ub = reinterpret_cast<const uint8_t *>(buffer);
  if (ub[0] == 0xFF && ub[1] == 0xD8 && ub[2] == 0xFF &&
    (ub[3] == 0xE0 || ub[3] == 0xE1 || ub[3] == 0xEE || ub[3] == 0xDB)) {
    return true;
  }
  return false;
}

bool IsBMPFormat(const char *buffer) {
  if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
    return true;
  }
  return false;
}

bool IsPFMFormat(const char *buffer) {
  if (buffer[0] == 'P' && (buffer[1] == 'f' || buffer[1] == 'F')) {
    return true;
  }
  return false;
}

bool IsPPMFormat(const char *buffer) {
  if (buffer[0] == 'P' && (buffer[1] == '5' || buffer[1] == '6')) {
    return true;
  }
  return false;
}

}

namespace mineola {
namespace imgpp {

bool Load(const char *fn, Img &img) {
  return Load(fn, img, false);
}

bool Load(const char *fn, Img &img, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn)) {
    return false;
  }

  std::string filename(fn);
  std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

  if (0 == filename.compare(filename.size() - 3, 3, "bmp")) {
    return LoadBMP(fn, img, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "jpg")
       || 0 == filename.compare(filename.size() - 4, 4, "jpeg")) {
    return LoadJPEG(fn, img, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "png")) {
    return LoadPNG(fn, img, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "pfm")) {
    return LoadPFM(fn, img, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "ppm")) {
    return LoadPPM(fn, img, bottom_first);
  } else if (0 == filename.compare(filename.size() - 4, 4, "bson")) {
    // ignores the bottom_first flag
    return LoadBSON(fn, img);
  } else {
    return false;
  }
}

bool Load(const char *buffer, uint32_t length, Img &img) {
  if (nullptr == buffer || 0 == length) {
    return false;
  }

  return Load(buffer, length, img, false);
}

bool Load(const char *buffer, uint32_t length, Img &img, bool bottom_first) {
  if (nullptr == buffer || 0 == length) {
    return false;
  }

  if (IsBMPFormat(buffer)) {
    return LoadBMP(buffer, length, img, bottom_first);
  } else if (IsJPEGFormat(buffer)) {
    return LoadJPEG(buffer, length, img, bottom_first);
  } else if (IsPNGFormat(buffer)) {
    return LoadPNG(const_cast<char *>(buffer), length, img, bottom_first);
  } else if (IsPFMFormat(buffer)) {
    return LoadPFM(buffer, length, img, bottom_first);
  } else if (IsPPMFormat(buffer)) {
    return LoadPPM(buffer, length, img, bottom_first);
  } else {
    return false;
  }
}

bool Write(const char *fn, const ImgROI &roi) {
  return Write(fn, roi, false);
}

bool Write(const char *fn, const ImgROI &roi, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn))
    return false;

  std::string filename(fn);
  std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

  if (0 == filename.compare(filename.size() - 3, 3, "bmp")) {
    return WriteBMP(fn, roi, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "jpg")
       || 0 == filename.compare(filename.size() - 4, 4, "jpeg")) {
    return WriteJPEG(fn, roi, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "png")) {
    return WritePNG(fn, roi, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "pfm")) {
    return WritePFM(fn, roi, bottom_first);
  } else if (0 == filename.compare(filename.size() - 3, 3, "ppm")) {
    return WritePPM(fn, roi, bottom_first);
  } else if (0 == filename.compare(filename.size() - 4, 4, "bson")) {
    return WriteBSON(fn, roi);
  } else {
    return false;
  }
}

}} // namespaces
