#ifndef MINEOLA_TEXTURE_HELPER
#define MINEOLA_TEXTURE_HELPER
#include <string>

namespace imgpp {
  class Img;
}

namespace mineola {
  struct TextureDesc;

  namespace texture_helper {

    /**
    * Convenience function to create a texture directly from a file on disk.
    * @param  texture_name - Texture name in engine resource map.
    * @param  filename     - Image path on disk.
    * @param  mipmap       - Whether to create mipmaps or not. (ignored if source is dds/ktx).
    * @return              - Creation is successful or not.
    */
    bool CreateTextureFromFile(const char *texture_name, const char *filename, bool mipmap);

    using texture_loader_t = std::add_pointer<bool(const char*, imgpp::Img &img)>::type;
    bool CreateTextureFromExtLoader(
      const char *texture_name, const char *fn,  bool mipmap, bool srgb,
      texture_loader_t loader);

    using texture_mem_loader_t =
      std::add_pointer<bool(const char*, uint32_t, imgpp::Img &img)>::type;
    bool CreateTextureFromExtLoader(
      const char *texture_name, const char *buffer,
      uint32_t length, bool mipmap, bool srgb,
      texture_mem_loader_t loader);

    /**
    * Convenience function to create a texture directly from a file on disk.
    * @param  texture_name - Texture name in engine resource map.
    * @param  buffer       - pointer to image data.
    * @param  length       - image data length in bytes.
    * @param  mipmap       - Whether to create mipmaps or not. (ignored if source is dds/ktx).
    * @return              - Creation is successful or not.
    */
    bool CreateTextureFromMemory(const char *texture_name, const void *buffer, size_t length, bool mipmap);

    /**
    * Split the texture creation to two steps. This is the first.
    * Load texture data from disk to main memory and create a TextureDesc data structure.
    * @param  filename - Image path on disk.
    * @param  mipmap   - Whether to create mipmaps or not. (ingored if source is dds/ktx).
    * @param  desc     - Output TextureDesc data structure.
    * @return          - Success or not.
    */
    bool CreateDescFromFile(const char *filename, bool mipmap, TextureDesc &desc);
    bool CreateDescFromMemory(const void *buffer, size_t length, bool mipmap, TextureDesc &desc);

    /**
     * Split the texture creation to two steps. This is the second.
     * Create texture and copy data to graphics memory given a TextureDesc.
     * @param  texture_name - Texture name in engine resource map.
     * @param  desc         - Input TextureDesc.
     * @return              - Success or not.
     */
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
