#ifndef IMGPP_GLHELPER_HPP
#define IMGPP_GLHELPER_HPP

/*! \file glhelper.hpp */

#include <mineola/ImgppTextureDesc.hpp>

namespace mineola {
namespace imgpp { namespace gl {
/*! \enum Profile
    \brief Specifies gl context version
*/
enum Profile: uint8_t {
  ES20, // no sRGB
  ES30, // has swizzle
  GL32,
  GL33, // has swizzle
};

/*! \struct GLFormatDesc
   Specifies texture format, data type in OpenGL
*/
struct GLFormatDesc {
  uint16_t internal_format; /*!< For compressed textures, equals the compressed internal format.
For uncompressed textures, specifies the internalformat parameter passed to glTexStorage*D or glTexImage*D */
  uint16_t external_format; /*!< For compressed textures, euqals 0. For uncompressed textures,
specifies the format parameter passed to glTex{, Sub}Image*D, usually one of {GL_RGB, GL_RGBA, GL_BGRA, etc.}*/
  uint16_t base_internal_format; /*!< Specifies base internal format of texture*/
  uint16_t type; /*!< For compressed textures, equals 0. For uncompressed textures, specifies the type
parameter passed to glTex{, Sub}Image*D, usually one of {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5, etc.}*/
};

TextureFormat TranslateFromGL(uint16_t internal, uint16_t external,
  uint16_t base_internal, uint16_t type);
GLFormatDesc TranslateToGL(TextureFormat format, Profile profile);
GLFormatDesc TranslateToGL(TextureFormat format);
uint32_t GetTypeSize(uint16_t type);

}}}  // namespaces

#endif
