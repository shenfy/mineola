#ifndef MINEOLA_STBIMAGEPLUGIN_H
#define MINEOLA_STBIMAGEPLUGIN_H

#ifdef MINEOLA_USE_STBIMAGE
#include <cstdint>

namespace imgpp {
  class Img;
}

namespace mineola {
  bool STBLoadImageFromFile(const char *fn, imgpp::Img &img);
  bool STBLoadImageFromMem(const char *buffer, uint32_t length, imgpp::Img &img);
}  // end namespace

#endif  // MINEOLA_USE_STBIMAGE

#endif
