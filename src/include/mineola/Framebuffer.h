#ifndef MINEOLA_FRAMEBUFFER_H
#define MINEOLA_FRAMEBUFFER_H

#include <vector>
#include <unordered_map>
#include "BasisObj.h"
#include "Texture.h"

namespace mineola {

class GraphicsBuffer;
class Viewport;

class Framebuffer : public Resource {
public:
  enum {AT_DEPTH = 0,
    AT_COLOR0 = 1, AT_COLOR1 = 2, AT_COLOR2 = 3, AT_COLOR3 = 4,
    AT_COLOR4 = 5, AT_COLOR5 = 6, AT_COLOR6 = 7, AT_COLOR7 = 8 };

  Framebuffer();
  virtual ~Framebuffer();

  void Bind();
  void Unbind();
  void SetAsRenderTarget();

  void AddViewport(std::shared_ptr<Viewport> viewport_ptr);
  void RemoveViewport(uint32_t id);
  std::shared_ptr<Viewport> &GetViewport(uint32_t id);
  uint32_t NumViewport() const;
  uint32_t Handle() const;
  const TextureDesc &GetTextureDesc(uint32_t attach_point);

protected:
  uint32_t fbo_;

  std::unordered_map<uint32_t, std::shared_ptr<Texture>> color_textures_;
  std::vector<uint32_t> attach_points_;
  std::shared_ptr<Texture> depth_texture_;
  std::vector<std::shared_ptr<Viewport>> viewports_;
  uint32_t width_, height_;
};

class ExternalFramebuffer : public Framebuffer {
public:
  ExternalFramebuffer();
  virtual ~ExternalFramebuffer();

  void SetParams(uint32_t fbo_id, uint32_t width, uint32_t height);

  uint32_t Width() const;
  uint32_t Height() const;
};

class InternalFramebuffer : public Framebuffer {
public:
  explicit InternalFramebuffer(size_t num_pbos);
  ~InternalFramebuffer();

  void AttachTexture(uint32_t attach_point, const char *texture_name, uint32_t level = 0);
  void AttachTexture(uint32_t attach_point,
    std::shared_ptr<Texture> texture_ptr, uint32_t level = 0);

  void ReadBack(uint32_t attach_point); //async read framebuffer to PBO
  GraphicsBuffer &GetReadPBO();
  GraphicsBuffer &GetPBO(size_t idx);

  void Blit(uint32_t fbo, int srcX0, int srcY0, int srcX1, int srcY1,
    int dstX0, int dstY0, int dstX1, int dstY1,
    unsigned int mask, unsigned int filter
  );

  uint32_t CheckStatus();

protected:
  size_t num_pbos_;
  size_t pbo_read_head_;
  size_t pbo_write_head_;
  std::vector<std::shared_ptr<GraphicsBuffer>> pbos_; //read back buffer

  uint32_t bpp_, array_size_;
};

} //end namespace

#endif
