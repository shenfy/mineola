#include "prefix.h"
#include <mineola/TextureHelper.h>
#include <cstring>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <mineola/ImgppLoaders.hpp>
#include <mineola/ImgppGLHelper.hpp>
#include <mineola/glutility.h>
#include <mineola/TypeMapping.h>
#include <mineola/Texture.h>
#include <mineola/Engine.h>
#include <mineola/PixelType.h>
#include <mineola/Framebuffer.h>
#include <mineola/ImgppTextureSrc.h>
#include <mineola/ReservedTextureUnits.h>

namespace {

using namespace mineola;
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

bool IsBMPFormat(const uint8_t *buffer) {
  if (buffer[0] == 0x42 && buffer[1] == 0x4D) {
    return true;
  }
  return false;
}

bool IsPFMFormat(const uint8_t *buffer) {
  if (buffer[0] == 'P' && (buffer[1] == 'f' || buffer[1] == 'F')) {
    return true;
  }
  return false;
}

bool IsPPMFormat(const uint8_t *buffer) {
  if (buffer[0] == 'P' && (buffer[1] == '5' || buffer[1] == '6')) {
    return true;
  }
  return false;
}

enum { kImgFmtUnknown = 0,
  kImgFmtDDS,
  kImgFmtKTX,
  kImgFmtJPEG,
  kImgFmtPNG,
  kImgFmtBMP,
  kImgFmtPFM,
  kImgFmtPPM
};

int DetermineImageFormat(const uint8_t *buffer) {
  if (IsDDSFormat(buffer)) {
    return kImgFmtDDS;
  } else if (IsKTXFormat(buffer)) {
    return kImgFmtKTX;
  } else if (IsJPEGFormat(buffer)) {
    return kImgFmtJPEG;
  } else if (IsPNGFormat(buffer)) {
    return kImgFmtPNG;
  } else if (IsBMPFormat(buffer)) {
    return kImgFmtBMP;
  } else if (IsPFMFormat(buffer)) {
    return kImgFmtPFM;
  } else if (IsPPMFormat(buffer)) {
    return kImgFmtPPM;
  } else {
    return kImgFmtUnknown;
  }
}

int PeekImageFormat(const char *fn) {
  enum {kMagicNumberDigits = 16};

  std::ifstream infile(fn, std::ios::binary);
  if (infile.good()) {
    infile.seekg(0, std::ios::end);
    auto file_size = infile.tellg();
    if (file_size < kMagicNumberDigits) {
      infile.close();
      return kImgFmtUnknown;
    }

    infile.seekg(0, std::ios::beg);
    std::string buffer;
    buffer.resize(kMagicNumberDigits);
    infile.read(&buffer[0], kMagicNumberDigits);
    infile.close();

    return DetermineImageFormat((const uint8_t*)buffer.data());
  }
  return kImgFmtUnknown;
}

}  // namespace

namespace mineola { namespace texture_helper {

std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const char *fn, bool bottom_first) {
  if (fn == nullptr || strlen(fn) == 0) {
    return {};
  }
  std::string found_fn;
  if (!Engine::Instance().ResrcMgr().LocateFile(fn, found_fn)) {
    MLOG("[%s] does not exists!\n", fn);
    return {};
  }

  auto img_fmt = PeekImageFormat(found_fn.c_str());
  std::shared_ptr<ImgppTextureSrc> tex_src;
  if (kImgFmtKTX == img_fmt) {
    imgpp::CompositeImg img;
    std::unordered_map<std::string, std::string> unused;
    if (imgpp::LoadKTX(found_fn.c_str(), img, unused, bottom_first)) {
      tex_src = CreateTextureSrc(img);
    }
  } else {
    imgpp::Img img;
    if (kImgFmtBMP == img_fmt) {
      if (imgpp::LoadBMP(found_fn.c_str(), img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtPPM == img_fmt) {
      if (imgpp::LoadPPM(found_fn.c_str(), img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtPFM == img_fmt) {
      if (imgpp::LoadPFM(found_fn.c_str(), img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtJPEG == img_fmt || kImgFmtPNG == img_fmt) {
      if (auto loader = Engine::Instance().ExtTextureLoader(); loader != nullptr) {
        if (loader(found_fn.c_str(), bottom_first, img)) {
          tex_src = CreateTextureSrc(img);
        }
      }
    }
  }
  return tex_src;
}

std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(
  const char *buffer,
  uint32_t length,
  bool bottom_first) {

  if (buffer == nullptr || length == 0) {
    return {};
  }
  auto img_fmt = DetermineImageFormat((const uint8_t*)buffer);
  std::shared_ptr<ImgppTextureSrc> tex_src;
  if (kImgFmtKTX == img_fmt) {
    // tex_src = LoadKTXFromMem(buffer, length);
    // TODO: load ktx from memory
    imgpp::CompositeImg img;
    std::unordered_map<std::string, std::string> unused;
    if (imgpp::LoadKTX(buffer, length, img, unused, bottom_first)) {
      tex_src = CreateTextureSrc(img);
    }
  } else {
    imgpp::Img img;
    if (kImgFmtBMP == img_fmt) {
      if (imgpp::LoadBMP(buffer, length, img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtPPM == img_fmt) {
      if (imgpp::LoadPPM(buffer, length, img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtPFM == img_fmt) {
      if (imgpp::LoadPFM(buffer, length, img, bottom_first)) {
        tex_src = CreateTextureSrc(img);
      }
    } else if (kImgFmtJPEG == img_fmt || kImgFmtPNG == img_fmt) {
      if (auto loader = Engine::Instance().ExtTextureMemLoader(); loader != nullptr) {
        if (loader(buffer, length, bottom_first, img)) {
          tex_src = CreateTextureSrc(img);
        }
      }
    }
  }
  return tex_src;
}

bool CreateTexture(
  const char *texture_name, const char *fn,
  bool bottom_first, bool mipmap, bool srgb,
  uint32_t min_filter, uint32_t mag_filter, uint32_t wrap_s, uint32_t wrap_t) {

  if (texture_name == nullptr || strlen(texture_name) == 0) {
    return false;
  }

  auto tex_src = CreateTextureSrc(fn, bottom_first);
  if (!tex_src) {
    return false;
  }

  TextureDesc desc;
  return CreateTextureDesc(tex_src,
    srgb, mipmap, min_filter, mag_filter, wrap_s, wrap_t,
    desc)
    && CreateTextureFromDesc(texture_name, desc);
}

bool CreateTexture(
  const char *texture_name, const char *buffer, uint32_t length,
  bool bottom_first, bool mipmap, bool srgb,
  uint32_t min_filter, uint32_t mag_filter, uint32_t wrap_s, uint32_t wrap_t) {
  if (texture_name == nullptr || strlen(texture_name) == 0) {
    return false;
  }

  auto tex_src = CreateTextureSrc(buffer, length, bottom_first);
  if (!tex_src) {
    return false;
  }

  TextureDesc desc;
  return CreateTextureDesc(tex_src,
    srgb, mipmap, min_filter, mag_filter, wrap_s, wrap_t,
    desc)
    && CreateTextureFromDesc(texture_name, desc);
}

std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const imgpp::Img &img) {
  // this is an uncompressed texture, format doesn't matter
  auto tex_src = std::make_shared<ImgppTextureSrc>(1, 1, 1, imgpp::FORMAT_UNDEFINED);
  tex_src->AddBuffer(img.Data());
  tex_src->AddROI(img.ROI());
  return tex_src;
}

std::shared_ptr<ImgppTextureSrc> CreateTextureSrc(const imgpp::CompositeImg &img) {
  if (img.Layers() != 1 || img.Faces() != 1) {
    // OpenGL wants continuous multi-layer texture data, which is not guarenteed in CompositeImg
    // mineola doesn't support cube map texture
    return nullptr;
  }

  auto tex_src = std::make_shared<ImgppTextureSrc>(
    img.Faces(), img.Layers(), img.Levels(), img.TexDesc().format);
  for (auto &buf: img.Buffers()) {
    tex_src->AddBuffer(buf);
  }
  if (img.IsCompressed()) {
    for (uint32_t level = 0; level < img.Levels(); ++level) {
      tex_src->AddBCROI(img.BlockROI(level, 0, 0));
    }
  } else {
    for (uint32_t level = 0; level < img.Levels(); ++level) {
      tex_src->AddROI(img.ROI(level, 0, 0));
    }
  }
  return tex_src;
}

bool CreateTextureDesc(std::shared_ptr<ImgppTextureSrc> tex_src,
  bool srgb, bool mipmap,
  uint32_t min_filter, uint32_t mag_filter,
  uint32_t wrap_s, uint32_t wrap_t,
  TextureDesc &desc) {

  auto dims = tex_src->Dimensions(0);
  if (tex_src->Faces() == 1 && tex_src->Layers() == 1) {
    if (dims[2] > 1) {
      desc.type = GL_TEXTURE_3D;
    } else {
      desc.type = GL_TEXTURE_2D;
    }
    desc.array_size = 1;
  } else {
    if (tex_src->Layers() > 1) {
      if (tex_src->Faces() > 1) {
        // GL_TEXTURE_CUBE_MAP_ARRAY does not suppported in GLES
        return false;
      } else {
        desc.array_size = dims[3];
        desc.type = GL_TEXTURE_2D_ARRAY;
      }
    } else {
      desc.type = GL_TEXTURE_CUBE_MAP;
      desc.array_size = 1;
    }

  }
  desc.compressed = tex_src->IsCompressed();
  desc.min_filter = min_filter;
  desc.mag_filter = mag_filter;
  desc.wrap_s = wrap_s;
  desc.wrap_t = wrap_t;
  desc.access = 0;

  // 0 means auto generate mipmaps
  desc.levels = mipmap ? (tex_src->Levels() > 1 ? tex_src->Levels() : 0) : 1;
  desc.depth_compare = false;
  desc.width = dims[0];
  desc.height = dims[1];
  desc.depth = dims[2];

  if (desc.compressed) {
    auto gl_format = imgpp::gl::TranslateToGL(tex_src->Format(), imgpp::gl::Profile::ES30);
    desc.internal_format = gl_format.internal_format;
  } else {
    const auto &roi = tex_src->ROI(0);
    pixel_type::Map2GL(roi.BPC(), roi.Channel(), roi.IsSigned(), roi.IsFloat(), srgb,
      desc.internal_format, desc.format, desc.data_type);
  }

  // check automatic mipmap generation capability for current internal format
  if (desc.levels == 0 && !pixel_type::Mipmappable(desc.internal_format)) {
    desc.levels = 1;
  }

  desc.src_data = std::move(tex_src);
  return true;
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
  auto tex_src = std::make_shared<ImgppTextureSrc>(1, 1, 1, imgpp::FORMAT_RGBA8_UNORM_PACK8);
  if (!tex_src) {
    return false;
  }
  tex_src->AddBuffer(img.Data());
  tex_src->AddROI(img.ROI());

  TextureDesc desc;
  return CreateTextureDesc(tex_src, false, false,
    TextureDesc::kLinearMipmapLinear, TextureDesc::kLinear,
    TextureDesc::kRepeat, TextureDesc::kRepeat,
    desc)
    && CreateTextureFromDesc("mineola:texture:fallback", desc);
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
  desc.min_filter = TextureDesc::kLinearMipmapLinear;
  desc.mag_filter = TextureDesc::kLinear;
  desc.wrap_s = TextureDesc::kClampToEdge;
  desc.wrap_t = TextureDesc::kClampToEdge;
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
  desc.min_filter = TextureDesc::kLinearMipmapLinear;
  desc.mag_filter = TextureDesc::kLinear;
  desc.wrap_s = TextureDesc::kRepeat;
  desc.wrap_t = TextureDesc::kRepeat;
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
  // create render target
  if (!CreateRenderTarget("mineola:rt:shadowmap",
    width, height, 32, false, GL_RED, GL_R8, GL_UNSIGNED_BYTE, 1, 1, true)) {
    return false;
  }

  // find depth texture
  auto depth_texture = bd_cast<Texture>(
    Engine::Instance().ResrcMgr().Find("mineola:rt:shadowmap:depth_texture"));
  if (!depth_texture) {
    return false;
  }

  // bind to texture unit
  glActiveTexture(GL_TEXTURE0 + kShadowmap0TextureUnit);
  depth_texture->Bind();
  glActiveTexture(GL_TEXTURE0 + kNumReservedTextureUnits);
  CHKGLERR_RET;
  return true;
}


}} //end namespace
