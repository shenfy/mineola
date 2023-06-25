#ifndef MINEOLA_TEXTURE_HELPER
#define MINEOLA_TEXTURE_HELPER
#include <string>
#include <memory>
#include <unordered_map>
#include <mineola/Imgpp.hpp>
#include <mineola/ImgppCompositeImg.hpp>
#include <mineola/Texture.h>

namespace mineola {

namespace imgpp {
  class Img;
}

struct TextureDesc;
class ImgppTextureSrc;

namespace texture_helper {

// Create texture using external loaders
bool CreateTexture(const char *texture_name, const char *fn,
  bool bottom_first, bool mipmap, bool srgb,
  uint32_t min_filter, uint32_t mag_filter, uint32_t wrap_s, uint32_t wrap_t);

// Create texture from memory
using mem_loader_t =
  std::add_pointer<bool(const char*, uint32_t, imgpp::Img &img)>::type;

bool CreateTexture(const char *texture_name, const char *buffer, uint32_t length,
  bool bottom_first, bool mipmap, bool srgb,
  uint32_t min_filter, uint32_t mag_filter, uint32_t wrap_s, uint32_t wrap_t);


std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const char *fn, bool bottom_first);
std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const char *buffer, uint32_t length, bool bottom_first);
std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const imgpp::Img &img);
std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const imgpp::CompositeImg &img);

bool CreateTextureDesc(std::shared_ptr<ImgppTextureSrc> tex_src,
  bool srgb, bool mipmap,
  uint32_t min_filter, uint32_t mag_filter,
  uint32_t wrap_s, uint32_t wrap_t,
  TextureDesc &desc);

bool CreateTextureFromDesc(const char *texture_name, const TextureDesc &desc);

bool CreateFallbackTexture2D();
bool CreateDepthTexture(const char *texture_name, uint32_t width, uint32_t height,
  uint32_t depth_bits, bool stencil, uint32_t samples = 1,
  bool depth_compare = false);
bool CreateEmptyTexture(const char *texture_name, uint32_t type,
  uint32_t width, uint32_t height, uint32_t depth,
  uint32_t array_size, uint32_t levels,
  uint32_t format, uint32_t internal_format,
  uint32_t data_type, uint32_t samples = 1);
bool CreateRenderTarget(const char *name,
  uint32_t width, uint32_t height,
  uint32_t depth_bits, bool stencil,
  uint32_t color_format, uint32_t internal_format,
  uint32_t color_dtype, uint32_t samples = 1,
  size_t num_pbos = 1, bool depth_compare = false);

bool CreateRenderTarget(const char *name,
  uint32_t width, uint32_t height,
  uint32_t depth_bits, bool stencil,
  uint32_t color_tex_count, uint32_t *color_format,
  uint32_t *internal_format, uint32_t *color_dtype,
  size_t num_pbos = 1, bool depth_compare = false);

bool CreateShadowmapRenderTarget(uint32_t width, uint32_t height);

}} //end namespace

#endif
