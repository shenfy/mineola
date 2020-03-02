#include "prefix.h"
#include "../include/TextureHelper.h"
#include <cstring>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <gli/gli.hpp>
#include "../include/glutility.h"
#include "../include/TypeMapping.h"
#include "../include/Texture.h"
#include "../include/Engine.h"
#include "../include/PixelType.h"
#include "../include/Framebuffer.h"
#include "../include/ImgppTextureSrc.h"
#include "../include/GLITextureSrc.h"

namespace {

using namespace mineola;

bool CreateTextureDescImgpp(const imgpp::Img &img, bool mipmap, bool srgb,
  TextureDesc &desc, uint32_t minimum_dimension) {

  const imgpp::ImgROI &roi = img.ROI();
  uint32_t dimension = 1;
  if (roi.Height() > 1) {
    dimension = 2;
  }
  if (roi.Depth() > 1) {
    dimension = 3;
  }
  dimension = std::max(dimension, minimum_dimension);
  if (dimension == 2) {
    desc.type = GL_TEXTURE_2D;
  } else if (dimension == 3) {
    desc.type = GL_TEXTURE_3D;
  } else {
    return false;
  }

  desc.compressed = false;
  desc.min_filter = GL_NEAREST_MIPMAP_LINEAR;
  desc.mag_filter = GL_LINEAR;
  desc.wrap_s = GL_REPEAT;
  desc.wrap_t = GL_REPEAT;
  desc.access = 0;
  desc.array_size = 1;
  desc.levels = mipmap ? 0 : 1;
  desc.depth_compare = false;

  pixel_type::Map2GL(roi.BPC(), roi.Channel(), roi.IsSigned(), roi.IsFloat(), srgb,
      desc.internal_format, desc.format, desc.data_type);
  desc.width = roi.Width();
  desc.height = roi.Height();
  desc.depth = roi.Depth();

  ImgppTextureSrc *data = new ImgppTextureSrc;
  if (data == nullptr)
      return false;
  data->AddBuffer(img.Data());
  data->AddROI(roi);
  desc.src_data.reset(data);

  return true;
}

bool CreateTextureDescGLI(const gli::texture &gli_texture, TextureDesc &desc) {

  gli::gl gli_gl(gli::gl::PROFILE_ES30);  // use highest version profile available
  const gli::gl::format texture_gl_format =
      gli_gl.translate(gli_texture.format(), gli_texture.swizzles());
  uint32_t target = gli_gl.translate(gli_texture.target());

  // rejects texture formats not implemented in mineola
  if (target == GL_TEXTURE_CUBE_MAP)
    return false;

  desc.type = target;
  desc.compressed = gli::is_compressed(gli_texture.format());
  desc.min_filter = GL_NEAREST_MIPMAP_LINEAR;
  desc.mag_filter = GL_LINEAR;
  desc.wrap_s = GL_REPEAT;
  desc.wrap_t = GL_REPEAT;
  desc.access = 0;
  switch (target) {
    case GL_TEXTURE_2D_ARRAY:
      desc.array_size = (uint32_t)(gli_texture.layers() * gli_texture.faces());
      break;
    default:
      desc.array_size = 1;
  }
  desc.levels = (uint32_t)gli_texture.levels();
  desc.format = texture_gl_format.External;
  desc.internal_format = texture_gl_format.Internal;
  desc.data_type = texture_gl_format.Type;
  desc.width = gli_texture.extent().x;
  desc.height = gli_texture.extent().y;
  desc.depth_compare = false;

  GLITextureSrc *data = new GLITextureSrc;
  if (data == nullptr)
    return false;
  data->SetGLITexture(gli_texture);
  desc.src_data.reset(data);

  return true;
}

bool CreateTextureDescGLIFile(const char *filename, TextureDesc &desc) {
  std::string found_path;
  if (!Engine::Instance().ResrcMgr().LocateFile(filename, found_path))
    return false;

  gli::texture gli_texture = gli::load(found_path);
  if (gli_texture.empty())
    return false;

  return CreateTextureDescGLI(gli_texture, desc);
}

bool CreateTextureDescGLIMem(const char *buffer, size_t length, TextureDesc &desc) {
  gli::texture gli_texture = gli::load(buffer, length);
  if (gli_texture.empty())
    return false;

  return CreateTextureDescGLI(gli_texture, desc);
}

bool IsDDSFormat(const char *buffer) {
  const uint32_t *value = (const uint32_t *)buffer;
  return (*value == 0x20534444);
}

bool IsKTXFormat(const char *buffer) {
  if ((uint8_t)buffer[0] == 0xAB && buffer[1] == 0x4B && buffer[2] == 0x54 && buffer[3] == 0x58
    && buffer[4] == 0x20 && buffer[5] == 0x31 && buffer[6] == 0x31 && (uint8_t)buffer[7] == 0xBB
    && buffer[8] == 0x0D && buffer[9] == 0x0A && buffer[10] == 0x1A && buffer[11] == 0x0A) {
    return true;
  }
  return false;
}

// bool IsPNGFormat(const char *buffer) {
//   if (buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47 &&
//     buffer[4] == 0x0D && buffer[5] == 0x0A && buffer[6] == 0x1A && buffer[7] == 0x0A) {
//     return true;
//   }
//   return false;
// }

// bool IsJPEGFormat(const char *buffer) {
//   if (buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF && buffer[3] == 0xE0) {
//     return true;
//   }
//   return false;
// }

}  // namespace

namespace mineola { namespace texture_helper {

bool CreateTextureFromImgpp(const char *texture_name, const imgpp::Img &img,
  bool mipmap, bool srgb, uint32_t minimum_dimension) {
    if (texture_name == nullptr || strlen(texture_name) == 0)
        return false;

    TextureDesc desc;
    if (!CreateTextureDescImgpp(img, mipmap, srgb, desc, minimum_dimension))
        return false;

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
        std::cerr << "Error: unsupported texture target for specified imgpp" << std::endl;
        return false;
    }

    std::shared_ptr<Texture> texture(texture_ptr);
    if (texture->Create(desc)) {
        Engine::Instance().ResrcMgr().Add(texture_name, texture);
        return true;
    } else
        return false;
}

bool CreateTextureFromFile(const char *texture_name, const char *filename, bool mipmap) {
  if (texture_name == nullptr || filename == nullptr
    || strlen(texture_name) == 0 || strlen(filename) == 0)
    return false;

  TextureDesc desc;
  if (!CreateDescFromFile(filename, mipmap, desc))
    return false;

  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateTextureFromExtLoader(
  const char *texture_name, const char *fn, bool mipmap, bool srgb,
  texture_loader_t loader) {
	if (texture_name == nullptr || fn == nullptr || loader == nullptr
		|| strlen(texture_name) == 0 || strlen(fn) == 0) {
		return false;
	}

	imgpp::Img img;
	TextureDesc desc;
	if (!loader(fn, img) || !CreateTextureDescImgpp(img, mipmap, srgb, desc, 2)) {
		return false;
	}
	return CreateTextureFromDesc(texture_name, desc);
}

bool CreateTextureFromExtLoader(
  const char *texture_name, const char *buffer,
  uint32_t length, bool mipmap, bool srgb,
  texture_mem_loader_t loader) {
  if (texture_name == nullptr || buffer == nullptr || length == 0) {
    return false;
  }

  imgpp::Img img;
  TextureDesc desc;
  if (!loader(buffer, length, img) || !CreateTextureDescImgpp(img, mipmap, srgb, desc, 2)) {
    return false;
  }
  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateDescFromFile(const char *filename, bool mipmap, TextureDesc &desc) {
  if (filename == nullptr || strlen(filename) == 0)
    return false;

  bool result = false;
  if (boost::algorithm::ends_with(filename, "dds")
    || boost::algorithm::ends_with(filename, "ktx")) {
    result = CreateTextureDescGLIFile(filename, desc);
  }
  return result;
}

bool CreateTextureFromMemory(const char *texture_name, const void *buffer, size_t length, bool mipmap) {
  if (texture_name == nullptr || strlen(texture_name) == 0)
    return false;
  TextureDesc desc;
  if (!CreateDescFromMemory(buffer, length, mipmap, desc))
    return false;

  return CreateTextureFromDesc(texture_name, desc);
}

bool CreateDescFromMemory(const void *buffer, size_t length, bool mipmap, TextureDesc &desc) {
  if (buffer == nullptr || length == 0)
    return false;
  bool result = false;
  if (IsDDSFormat((const char *)buffer) || IsKTXFormat((const char *)buffer)) {
    result = CreateTextureDescGLIMem((const char *)buffer, length, desc);
  }
  return result;
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

  std::shared_ptr<Texture> texture(texture_ptr);
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
  desc.format = GL_RGB;
  desc.internal_format = GL_RGB;
  desc.data_type = GL_UNSIGNED_BYTE;
  desc.width = 1;
  desc.height = 1;
  desc.depth_compare = false;

  ImgppTextureSrc *data = new ImgppTextureSrc;
  if (data == nullptr)
    return false;
  data->AddBuffer(img.Data());
  data->AddROI(img.ROI());
  desc.src_data.reset(data);

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
