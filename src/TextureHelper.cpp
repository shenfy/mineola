#include "prefix.h"
#include "../include/TextureHelper.h"
#include <cstring>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <imgpp/bchelper.hpp>
#include "../include/glutility.h"
#include "../include/TypeMapping.h"
#include "../include/Texture.h"
#include "../include/Engine.h"
#include "../include/PixelType.h"
#include "../include/Framebuffer.h"
#include "../include/ImgppTextureSrc.h"
#include "../include/KTXImageLoader.h"

namespace {

using namespace mineola;

void CreateTextureSrcFromBitmapImgpp(const imgpp::Img &img, std::shared_ptr<ImgppTextureSrc> &tex_src) {
  tex_src = std::make_shared<ImgppTextureSrc>(0, 0, false, false);
  tex_src->AddBuffer(img.Data());
  tex_src->AddROI(img.ROI());
}

bool CreateTextureDescFromImgppTextureSrc(std::shared_ptr<ImgppTextureSrc> tex_src,
  bool srgb, bool mipmap, TextureDesc &desc) {
  auto dims = tex_src->Dimensions(0);
  if (dims[2] > 1 && dims[3] > 1) {
    return false;
  }
  if (dims[2] == 1) {
    if (dims[3] == 1) {
      desc.type = GL_TEXTURE_2D;
    } else {
      desc.type = GL_TEXTURE_2D_ARRAY;
    }
  } else {
    desc.type = GL_TEXTURE_3D;
  }
  desc.compressed = tex_src->IsCompressed();
  desc.min_filter = GL_NEAREST_MIPMAP_LINEAR;
  desc.mag_filter = GL_LINEAR;
  desc.wrap_s = GL_REPEAT;
  desc.wrap_t = GL_REPEAT;
  desc.access = 0;
  desc.array_size = dims[3];
  desc.levels = mipmap ? 0 : 1;
  desc.depth_compare = false;
  desc.width = dims[0];
  desc.height = dims[1];
  desc.depth = dims[2];

  if (desc.compressed) {
    desc.internal_format = imgpp::BCFormatToKHR(tex_src->BCROI(0).Format());
  } else {
    const auto &roi = tex_src->ROI(0);
    pixel_type::Map2GL(roi.BPC(), roi.Channel(), roi.IsSigned(), roi.IsFloat(), srgb,
      desc.internal_format, desc.format, desc.data_type);
  }
  desc.src_data = tex_src;
  return true;
}

bool IsDDSFormat(const uint8_t *buffer) {
  const uint32_t *value = (const uint32_t *)buffer;
  return (*value == 0x20534444);
}

// KTX version 1.1
bool IsKTXFormat(const uint8_t *buffer) {
  if ((uint8_t)buffer[0] == 0xAB && buffer[1] == 0x4B && buffer[2] == 0x54 && buffer[3] == 0x58
    && buffer[4] == 0x20 && buffer[5] == 0x31 && buffer[6] == 0x31 && (uint8_t)buffer[7] == 0xBB
    && buffer[8] == 0x0D && buffer[9] == 0x0A && buffer[10] == 0x1A && buffer[11] == 0x0A) {
    return true;
  }
  return false;
}

bool IsPNGFormat(const uint8_t *buffer) {
  if (buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47 &&
    buffer[4] == 0x0D && buffer[5] == 0x0A && buffer[6] == 0x1A && buffer[7] == 0x0A) {
    return true;
  }
  return false;
}

bool IsJPEGFormat(const uint8_t *buffer) {
  if (buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF &&
    (buffer[3] == 0xE0 || buffer[3] == 0xE1 || buffer[3] == 0xEE || buffer[3] == 0xDB)) {
    return true;
  }
  return false;
}

const char *DetermineImageFormat(const uint8_t *buffer) {
  if (IsDDSFormat(buffer)) {
    return "dds";
  } else if (IsKTXFormat(buffer)) {
    return "ktx";
  } else if (IsJPEGFormat(buffer)) {
    return "jpeg";
  } else if (IsPNGFormat(buffer)) {
    return "png";
  } else {
    return "?";
  }
}

const char *PeekImageFormat(const char *fn) {
  enum {kMagicNumberDigits = 16};

  std::ifstream infile(fn, std::ios::binary);
  if (infile.good()) {
    infile.seekg(0, std::ios::end);
    auto file_size = infile.tellg();
    if (file_size < kMagicNumberDigits) {
      infile.close();
      return "?";
    }

    infile.seekg(0, std::ios::beg);
    std::string buffer;
    buffer.resize(kMagicNumberDigits);
    infile.read(&buffer[0], kMagicNumberDigits);
    infile.close();

    return DetermineImageFormat((const uint8_t*)buffer.data());
  }
  return "?";
}

}  // namespace

namespace mineola { namespace texture_helper {

bool CreateTexture(
  const char *texture_name,
  const char *fn,
  bool mipmap, bool srgb) {

  if (texture_name == nullptr || fn == nullptr
    || strlen(texture_name) == 0 || strlen(fn) == 0) {
    return false;
  }

  TextureDesc desc;
  // determine file format
  auto img_fmt = PeekImageFormat(fn);
  if (img_fmt == "ktx") {
    std::shared_ptr<ImgppTextureSrc> tex_src;
    if (!LoadKTXFromFile(fn, tex_src)) {
      return false;
    }
    if (!CreateTextureDescFromImgppTextureSrc(std::move(tex_src), srgb, mipmap, desc)) {
      return false;
    }
  } else if (img_fmt == "jpeg" || img_fmt == "png") {
    if (auto loader = Engine::Instance().ExtTextureLoader(); loader != nullptr) {
      imgpp::Img img;
      if (!loader(fn, img)) {
        return false;
      }
      std::shared_ptr<ImgppTextureSrc> tex_src;
      CreateTextureSrcFromBitmapImgpp(img, tex_src);
      if (!CreateTextureDescFromImgppTextureSrc(std::move(tex_src), srgb, mipmap, desc)) {
        return false;
      }
    } else {
      return false;
    }
  }

  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateTexture(
  const char *texture_name,
  const char *buffer, uint32_t length,
  bool mipmap, bool srgb) {

  if (texture_name == nullptr || buffer == nullptr || length == 0) {
    return false;
  }

  TextureDesc desc;

  auto img_fmt = DetermineImageFormat((const uint8_t*)buffer);

  if (img_fmt == "ktx") {
    std::shared_ptr<ImgppTextureSrc> tex_src;
    if (!LoadKTXFromMem(buffer, length, tex_src)) {
      return false;
    }
    if (!CreateTextureDescFromImgppTextureSrc(std::move(tex_src), srgb, mipmap, desc)) {
      return false;
    }
  } else if (img_fmt == "jpeg" || img_fmt == "png") {
    if (auto loader = Engine::Instance().ExtTextureMemLoader(); loader != nullptr) {
      imgpp::Img img;
      if (!loader(buffer, length, img)) {
        return false;
      }
      std::shared_ptr<ImgppTextureSrc> tex_src;
      CreateTextureSrcFromBitmapImgpp(img, tex_src);
      if (!CreateTextureDescFromImgppTextureSrc(std::move(tex_src), srgb, mipmap, desc)) {
        return false;
      }
    } else {
      return false;
    }
  }
  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateTextureFromDesc(const char *texture_name, const TextureDesc &desc) {
  if (texture_name == nullptr || strlen(texture_name) == 0)
    return false;

  // check target type
  Texture *texture_ptr = nullptr;
  switch (desc.type) {
    case GL_TEXTURE_2D:
    case GL_TEXTURE_2D_ARRAY:
      texture_ptr = new Texture2D;
      break;
    case GL_TEXTURE_3D:
      texture_ptr = new Texture3D;
      break;
    default:
      break;
  }
  if (texture_ptr == nullptr) {  // unsupported texture target
    MLOG("Error: unsupported texture target for %s!\n", texture_name);
    return false;
  }

  auto texture = std::shared_ptr<Texture>(texture_ptr);
  if (texture->Create(desc)) {
    Engine::Instance().ResrcMgr().Add(texture_name, texture);
    return true;
  } else
    return false;
}

bool CreateFallbackTexture2D() {
  imgpp::Img img;
  img.SetSize(1, 1, 1, 4, 8);
  img.ROI().At<uint8_t>(0, 0, 0) = 255;
  img.ROI().At<uint8_t>(0, 0, 1) = 255;
  img.ROI().At<uint8_t>(0, 0, 2) = 255;
  img.ROI().At<uint8_t>(0, 0, 3) = 255;
  auto tex_src = std::make_shared<ImgppTextureSrc>(1, 1, false, false);
  if (!tex_src) {
    return false;
  }
  tex_src->AddBuffer(img.Data());
  tex_src->AddROI(img.ROI());

  TextureDesc desc;
  if (!CreateTextureDescFromImgppTextureSrc(std::move(tex_src), false, false, desc)) {
    return false;
  }

  return CreateTextureFromDesc("mineola:texture:fallback", desc);
}

bool CreateDepthTexture(const char *texture_name, uint32_t width, uint32_t height,
            uint32_t bits, bool stencil, uint32_t samples, bool depth_compare) {
  if (texture_name == 0 || strlen(texture_name) == 0)
    return false;

  if (samples != 1)
    return false;

  uint32_t internal_format = 0, data_type = 0;
  if (!type_mapping::Map2GLDepthFormat(bits, stencil, internal_format, data_type)) return false;

  TextureDesc desc;
  desc.type = GL_TEXTURE_2D;
  desc.compressed = false;
  desc.min_filter = GL_NEAREST_MIPMAP_LINEAR;
  desc.mag_filter = GL_LINEAR;
  desc.wrap_s = GL_REPEAT;
  desc.wrap_t = GL_REPEAT;
  desc.access = 0;
  desc.array_size = 1;
  desc.levels = 1;
  desc.format = GL_DEPTH_COMPONENT;
  desc.internal_format = internal_format;
  desc.data_type = data_type;
  desc.width = width;
  desc.height = height;
  desc.samples = samples;
  desc.depth_compare = depth_compare;

  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateEmptyTexture(const char *texture_name, uint32_t type,
            uint32_t width, uint32_t height, uint32_t depth,
            uint32_t array_size, uint32_t levels,
            uint32_t format, uint32_t internal_format,
            uint32_t data_type, uint32_t samples) {
  if (texture_name == 0 || strlen(texture_name) == 0)
    return false;

  TextureDesc desc;
  desc.type = type;
  desc.compressed = false;
  desc.min_filter = GL_NEAREST_MIPMAP_LINEAR;
  desc.mag_filter = GL_LINEAR;
  desc.wrap_s = GL_REPEAT;
  desc.wrap_t = GL_REPEAT;
  desc.access = 0;
  desc.array_size = array_size;
  desc.levels = levels;
  desc.format = format;
  desc.internal_format = internal_format;
  desc.data_type = data_type;
  desc.width = width;
  desc.height = height;
  desc.depth = depth;
  desc.samples = samples;
  desc.depth_compare = false;

  std::shared_ptr<Texture> texture;
  switch (type) {
    case GL_TEXTURE_2D:
    case GL_TEXTURE_2D_ARRAY:
      texture = std::make_shared<Texture2D>();
      break;
    case GL_TEXTURE_3D:
      texture = std::make_shared<Texture3D>();
      break;
    default:
      return false;
  }

  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateRenderTarget(const char *name,
            uint32_t width, uint32_t height,
            uint32_t depth_bits, bool stencil,
            uint32_t color_format, uint32_t internal_format,
            uint32_t color_dtype, uint32_t samples,
            size_t num_pbos,
            bool depth_compare) {
  if (samples != 1)
    return false;

  std::string depth_name = name;
  depth_name.append(":depth_texture");

  if (!CreateDepthTexture(depth_name.c_str(), width, height,
    depth_bits, stencil, samples, depth_compare))
    return false;

  auto &en = Engine::Instance();

  std::string color_name = name;
  color_name.append(":color_texture");

  uint32_t type = GL_TEXTURE_2D;
  if (!CreateEmptyTexture(color_name.c_str(), type, width, height, 1, 1, 1,
    color_format, internal_format, color_dtype, samples)) {
    en.ResrcMgr().Remove(depth_name);
    return false;
  }

  std::shared_ptr<InternalFramebuffer> fb(new InternalFramebuffer(num_pbos));
  fb->AttachTexture(Framebuffer::AT_COLOR0, color_name.c_str());
  fb->AttachTexture(Framebuffer::AT_DEPTH, depth_name.c_str());
  en.ResrcMgr().Add(name, bd_cast<Resource>(fb));

  auto status = fb->CheckStatus();
  const char *s = getFramebufferStatusString(status);
  if (s) {
    printf("Error: %s\n", s);
    return false;
  }
  return true;
}


bool CreateRenderTarget(const char *name,
            uint32_t width, uint32_t height,
            uint32_t depth_bits, bool stencil,
            uint32_t color_tex_count, uint32_t *color_format,
            uint32_t *internal_format, uint32_t *color_dtype,
            size_t num_pbos,
            bool depth_compare) {
  std::vector<std::string> created_resources;
  std::string depth_name = name;
  depth_name.append(":depth_texture");

  if (!CreateDepthTexture(depth_name.c_str(),
    width, height, depth_bits, stencil, 1, depth_compare))
    return false;
  created_resources.push_back(depth_name);

  auto &en = Engine::Instance();

  for (uint32_t i = 0; i < color_tex_count; ++i) {
    std::string color_name = name;
    color_name.append(":color_texture").append(std::to_string(i));

    if (!CreateEmptyTexture(color_name.c_str(), GL_TEXTURE_2D, width, height, 1, 1, 1,
      color_format[i], internal_format[i], color_dtype[i])) {
      for (const auto &rersc_name : created_resources)
        en.ResrcMgr().Remove(rersc_name);
      return false;
    }
    created_resources.push_back(color_name);
  }

  std::shared_ptr<InternalFramebuffer> fb(new InternalFramebuffer(num_pbos));
  for (uint32_t i = 0; i < color_tex_count; ++i)
    fb->AttachTexture(Framebuffer::AT_COLOR0 + i, created_resources[i + 1].c_str());
  fb->AttachTexture(Framebuffer::AT_DEPTH, depth_name.c_str());
  en.ResrcMgr().Add(name, bd_cast<Resource>(fb));

  auto status = fb->CheckStatus();
  const char *s = getFramebufferStatusString(status);
  if (s) {
    printf("Error: %s\n", s);
    return false;
  }
  return true;
}

bool CreateShadowmapRenderTarget(uint32_t width, uint32_t height) {
  return CreateRenderTarget("mineola:rt:shadowmap",
    width, height, 32, false, GL_RED, GL_R8, GL_UNSIGNED_BYTE, 1, 1, true);
}


}} //end namespace
