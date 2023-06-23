#ifndef MINEOLA_STBIMAGEPLUGIN_H
#define MINEOLA_STBIMAGEPLUGIN_H

#ifdef MINEOLA_USE_STBIMAGE
#include <cstdint>

namespace mineola {
  namespace imgpp {
    class Img;
  }

  bool STBLoadImageFromFile(const char *fn, bool bottom_first, imgpp::Img &img);
  bool STBLoadImageFromMem(const char *buffer, uint32_t length, bool bottom_first, imgpp::Img &img);
}  // end namespace

#endif  // MINEOLA_USE_STBIMAGE

#endif
