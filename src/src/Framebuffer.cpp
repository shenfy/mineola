#include "prefix.h"
#include <fstream>
#include <algorithm>
#include <imgpp/imgpp.hpp>
#include <mineola/glutility.h>
#include <mineola/Framebuffer.h>
#include <mineola/Engine.h>
#include <mineola/GraphicsBuffer.h>
#include <mineola/Viewport.h>

namespace mineola {

Framebuffer::Framebuffer() : fbo_(0), width_(0), height_(0) {
  //viewport[0] covers the entire framebuffer
  viewports_.push_back(std::make_shared<Viewport>());
}

Framebuffer::~Framebuffer() {}

uint32_t Framebuffer::Handle() const {
  return fbo_;
}

void Framebuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void Framebuffer::Unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::SetAsRenderTarget() {
  Bind();
  glDrawBuffers((GLsizei)attach_points_.size(), &attach_points_[0]);
  CHKGLERR
}

void Framebuffer::AddViewport(std::shared_ptr<Viewport> viewport_ptr) {
  //check for redundancy first
  auto iter = std::find(viewports_.begin(), viewports_.end(), viewport_ptr);
  if (iter == viewports_.end()) {  //not found
    if (color_textures_.size() != 0) {  //must have at least one color attachment!
      viewport_ptr->OnSize(width_, height_);
    }
    viewports_.push_back(std::move(viewport_ptr));
  }
}

void Framebuffer::RemoveViewport(uint32_t id) {
  if (viewports_.size() > id)
    viewports_.erase(viewports_.begin() + id);
}

uint32_t Framebuffer::NumViewport() const {
  return (uint32_t)viewports_.size();
}

std::shared_ptr<Viewport> &Framebuffer::GetViewport(uint32_t id) {
  static std::shared_ptr<Viewport> empty_ptr;
  if (viewports_.size() > id)
    return viewports_[id];
  else
    return empty_ptr;
}

const TextureDesc &Framebuffer::GetTextureDesc(uint32_t attach_point) {
  if (AT_DEPTH == attach_point) {
    return depth_texture_->Desc();
  } else {
    return color_textures_[attach_point]->Desc();
  }
}

ExternalFramebuffer::ExternalFramebuffer() {}

ExternalFramebuffer::~ExternalFramebuffer() {}

void ExternalFramebuffer::SetParams(uint32_t fbo_id, uint32_t width, uint32_t height) {
  fbo_ = fbo_id;
  width_ = width;
  height_ = height;
  // set default color attachment
  attach_points_.clear();
  #if defined(__ANDROID_API__) || defined(MINEOLA_DESKTOP)
  if (fbo_ == 0) {
    attach_points_.push_back(GL_BACK);
  } else {
    attach_points_.push_back(GL_COLOR_ATTACHMENT0);
  }
  #else
  attach_points_.push_back(GL_COLOR_ATTACHMENT0);
  #endif
  // create default viewport covering the entire framebuffer
  viewports_.clear();
  viewports_.push_back(std::make_shared<Viewport>());
}

uint32_t ExternalFramebuffer::Width() const {
  return width_;
}

uint32_t ExternalFramebuffer::Height() const {
  return height_;
}

InternalFramebuffer::InternalFramebuffer(size_t num_pbos) :
  num_pbos_(num_pbos), pbo_read_head_(0), pbo_write_head_(num_pbos - 1),
  bpp_(0) {

  glGenFramebuffers(1, &fbo_);

  for (size_t idx = 0; idx < num_pbos_; ++idx) {
    pbos_.push_back(std::make_shared<GraphicsBuffer>(
      GraphicsBuffer::STREAM, GraphicsBuffer::READ, GraphicsBuffer::READ_ONLY,
      GL_PIXEL_PACK_BUFFER));
  }
}

InternalFramebuffer::~InternalFramebuffer() {
  if (fbo_) {
    glDeleteFramebuffers(1, &fbo_);
    fbo_ = 0;
    pbos_.clear();
  }
}

void InternalFramebuffer::AttachTexture(uint32_t attach_point,
  const char *texture_name, uint32_t level) {
  std::shared_ptr<Texture> texture_ptr =
    bd_cast<Texture>(Engine::Instance().ResrcMgr().Find(texture_name));
  if (texture_ptr) {
    AttachTexture(attach_point, texture_ptr, level);
  }
}

void InternalFramebuffer::AttachTexture(uint32_t attach_point,
  std::shared_ptr<Texture> texture_ptr,
  uint32_t level) {

  if (texture_ptr) {
    Bind();
    if (AT_DEPTH == attach_point) {
      uint32_t bits;
      bool stencil;
      type_mapping::MapGLDepthFormat(texture_ptr->Desc().internal_format, bits, stencil);
      if (stencil) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
          texture_ptr->Handle(), level);
      } else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
          texture_ptr->Handle(), level);
      }
      depth_texture_ = std::move(texture_ptr);
    } else {
      glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + (attach_point - AT_COLOR0), GL_TEXTURE_2D,
        texture_ptr->Handle(), level);

      if (color_textures_.size() == 0) {  //first texture
        //resize PBOs
        if (pbos_.size() != 0) {
          const TextureDesc &desc = texture_ptr->Desc();
          bpp_ = type_mapping::NumChannels(desc.format) * type_mapping::SizeOfGLType(desc.data_type);
          width_ = desc.width >> level;
          height_ = desc.height >> level;
          array_size_ = desc.array_size;
          uint32_t num_channels = type_mapping::NumChannels(desc.format);
          uint32_t bpc = type_mapping::SizeOfGLType(desc.data_type) << 3;
          uint32_t pitch = imgpp::ImgROI::CalcPitch(width_, num_channels, bpc, 4);
          uint32_t pbo_size = pitch * height_ * array_size_;
          for (auto &pbo : pbos_) {
            pbo->SetSize(pbo_size);
          }
        }

        //resize viewports
        for (auto iter = viewports_.begin(); iter != viewports_.end(); ++iter)
          (*iter)->OnSize(width_, height_);
      }
      color_textures_[attach_point] = std::move(texture_ptr);
      attach_points_.push_back(GL_COLOR_ATTACHMENT0 + (attach_point - AT_COLOR0));
      std::sort(attach_points_.begin(), attach_points_.end());
    }
#if defined(__ANDROID_API__) || defined(MINEOLA_DESKTOP)
    Unbind();
#endif
    CHKGLERR
  }
}

void InternalFramebuffer::ReadBack(uint32_t attach_point) {
  if (pbos_.size() == 0)
    return;

  glReadBuffer(GL_COLOR_ATTACHMENT0 + attach_point - AT_COLOR0);
  pbos_[pbo_write_head_]->Bind();
  const TextureDesc &desc = color_textures_[attach_point]->Desc();
  glReadPixels(0, 0, width_, height_, desc.format, desc.data_type, 0); //read fb to pbo
  CHKGLERR

  pbo_write_head_ = (pbo_write_head_ + 1) % pbos_.size();
  pbo_read_head_ = (pbo_read_head_ + 1) % pbos_.size();
}

GraphicsBuffer &InternalFramebuffer::GetReadPBO() {
  return *pbos_[pbo_read_head_];
}

GraphicsBuffer &InternalFramebuffer::GetPBO(size_t idx) {
  return *pbos_[idx];
}

void InternalFramebuffer::Blit(uint32_t fbo, int srcX0, int srcY0, int srcX1, int srcY1,
    int dstX0, int dstY0, int dstX1, int dstY1,
    unsigned int mask, unsigned int filter) {
  GLint old_read_buffer, old_draw_buffer;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_read_buffer);
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_draw_buffer);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
    dstX0, dstY0, dstX1, dstY1,
    mask, filter);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, old_read_buffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, old_draw_buffer);
  CHKGLERR
}

uint32_t InternalFramebuffer::CheckStatus() {
  Bind();
  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
#if defined(__ANDROID_API__) || defined(MINEOLA_DESKTOP)
  Unbind();
#endif
  return status;
}

} //end namespace
