#include "prefix.h"
#include <mineola/Texture.h>
#include <algorithm>
#include <mineola/glutility.h>
#include <mineola/TextureTypes.h>

namespace {
  enum {kDefaultAlignment = 4};
}

namespace mineola {

Texture::Texture()
  : handle_(0) {
}

Texture::~Texture() {
}

void Texture::Bind() {
  glBindTexture(desc_.type, handle_);
}

uint32_t Texture::Handle() const {
  return handle_;
}

const TextureDesc &Texture::Desc() const {
  return desc_;
}

ExternalTexture::ExternalTexture(uint32_t handle, const TextureDesc &desc) {
  handle_ = handle;
  desc_ = desc;
}

ExternalTexture::~ExternalTexture() {

}

bool ExternalTexture::Create(const TextureDesc &desc) {
  return false;
}

///////////////////////////////////

InternalTexture::InternalTexture() {
  glGenTextures(1, &handle_);
}

InternalTexture::~InternalTexture() {
  if (handle_)
    glDeleteTextures(1, &handle_);
}

///////////////////////////////////

Texture2D::Texture2D() {
}

Texture2D::~Texture2D() {
}

bool Texture2D::Create(const TextureDesc &desc) {
  if (desc.type != GL_TEXTURE_2D && desc.type != GL_TEXTURE_2D_ARRAY) {
    return false;
  }

  // compressed texture must be initialized at creation time
  if (desc.compressed && !desc.src_data) {
    return false;
  }

  // mipmapped multisample texture not allowed
  if (desc.samples > 1 && desc.levels > 1) {
    return false;
  }

  desc_ = desc;
  glBindTexture(desc_.type, handle_);

  int actual_levels = desc_.levels;
  if (actual_levels == 0) {  // manually calculate max level
    actual_levels = (int)std::floor(std::log2((float)std::max(desc_.width, desc_.height))) + 1;
  }

  //setup sampling parameters
  if (desc_.type == GL_TEXTURE_2D || desc_.type == GL_TEXTURE_2D_ARRAY) {
    glTexParameteri(desc_.type, GL_TEXTURE_MAG_FILTER, gl::MapFilterMode(desc_.mag_filter));
    glTexParameteri(desc_.type, GL_TEXTURE_MIN_FILTER, gl::MapFilterMode(desc_.min_filter));
    glTexParameteri(desc_.type, GL_TEXTURE_WRAP_S, gl::MapWrapMode(desc_.wrap_s));
    glTexParameteri(desc_.type, GL_TEXTURE_WRAP_T, gl::MapWrapMode(desc_.wrap_t));
    glTexParameteri(desc_.type, GL_TEXTURE_MAX_LEVEL, actual_levels - 1);
    glTexParameteri(desc_.type, GL_TEXTURE_COMPARE_MODE,
      desc_.depth_compare ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
    glTexParameteri(desc_.type, GL_TEXTURE_COMPARE_FUNC, desc_.depth_func);
  }

  bool initialize = (bool)desc_.src_data; //is initial data provided?
  if (desc_.array_size == 1) {  //not a texture array
    if (desc_.levels == 0) {  //automatically generate mipmaps
      if (initialize) {
        glm::ivec3 dims = desc_.src_data->Dimensions(0);
        const void *data = desc_.src_data->Data(0, 0, 0);
        if (desc_.compressed) {
          uint32_t data_size = desc_.src_data->DataSize(0);
          glCompressedTexImage2D(desc_.type, 0, desc_.internal_format,
            dims[0], dims[1], 0, data_size, data);
        } else {
          int alignment = desc_.src_data->Alignment(0);
          glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
          glTexImage2D(desc_.type, 0, desc_.internal_format,
            dims[0], dims[1], 0, desc_.format, desc_.data_type, data);
          glGenerateMipmap(desc_.type);
          glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);  // resotre pack alignment
        }
      } else {
        glTexImage2D(desc_.type, 0, desc_.internal_format,
          desc.width, desc.height, 0, desc_.format, desc_.data_type, 0);
        glGenerateMipmap(desc_.type);
      }
    } else {  //precomputed mipmaps
      for (uint32_t level = 0; level < desc_.levels; ++level) {
        if (initialize) {
          glm::ivec3 dims = desc_.src_data->Dimensions(level);
          const void *data = desc_.src_data->Data(0, 0, level);
          if (desc_.compressed) {
            uint32_t data_size = desc_.src_data->DataSize(level);
            glCompressedTexImage2D(desc_.type, level, desc_.internal_format,
              dims[0], dims[1], 0, data_size, data);
          } else {
            int alignment = desc_.src_data->Alignment(level);
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            glTexImage2D(desc_.type, level, desc_.internal_format,
              dims[0], dims[1], 0, desc_.format, desc_.data_type, data);
          }
        } else {
          uint32_t width = desc_.width >> level; uint32_t height = desc_.height >> level;
          glTexImage2D(desc_.type, level, desc_.internal_format,
            width, height, 0, desc_.format, desc_.data_type, 0);
        }
      }
      if (initialize && !desc_.compressed) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);  // resotre pack alignment
      }
    }
  } else if (desc_.array_size > 1) {  //texture array
    if (desc_.levels == 0) {  //auto mipmaps
      if (initialize) {
        glm::ivec3 dims = desc_.src_data->Dimensions(0);
        const void *data = desc_.src_data->Data(0, 0, 0);
        if (desc_.compressed) {
          uint32_t data_size = desc_.src_data->DataSize(0);
          glCompressedTexImage3D(desc_.type, 0, desc_.internal_format,
            dims[0], dims[1], desc_.array_size, 0, data_size, data);
        } else {
          int alignment = desc_.src_data->Alignment(0);
          glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
          glTexImage3D(desc_.type, 0, desc_.internal_format,
            dims[0], dims[1], desc_.array_size, 0,
            desc_.format, desc_.data_type, data);
          glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);  // resotre pack alignment
        }
      } else {
        glTexImage3D(desc_.type, 0, desc_.internal_format,
          desc.width, desc.height, desc.array_size, 0, desc_.format,
          desc_.data_type, 0);
      }
      glGenerateMipmap(desc_.type);
    } else {
      for (uint32_t level = 0; level < desc_.levels; ++level) {
        if (initialize) {
          glm::ivec3 dims = desc_.src_data->Dimensions(level);
          const void *data = desc_.src_data->Data(0, 0, level);

          if (desc_.compressed) {
            uint32_t data_size = desc_.src_data->DataSize(level);
            glCompressedTexImage3D(desc_.type, level, desc_.internal_format,
              dims[0], dims[1], desc_.array_size, 0, data_size, data);
          } else {
            int alignment = desc_.src_data->Alignment(level);
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
            glTexImage3D(desc_.type, level, desc_.internal_format,
              dims[0], dims[1], desc_.array_size, 0,
              desc_.format, desc_.data_type, data);
          }
        } else {
          uint32_t width = desc_.width >> level;
          uint32_t height = desc_.height >> level;
          glTexImage3D(desc_.type, level, desc_.internal_format,
            width, height, desc_.array_size, 0,
            desc_.format, desc_.data_type, 0);
        }
      }
      if (initialize && !desc.compressed) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);
      }
    }
  }
  glBindTexture(desc_.type, 0);
  desc_.src_data.reset();
  CHKGLERR_RET

  return true;
}

bool Texture2D::UpdateTexture(const SubTextureDesc &sub_desc, const void *data) {
  if (handle_ == 0)
    return false;

  glBindTexture(desc_.type, handle_);

  if (sub_desc.x_offset + sub_desc.width > desc_.width
    || sub_desc.y_offset + sub_desc.height > desc_.height) {
    return false;
  }

  if (desc_.compressed) {
    glCompressedTexSubImage2D(desc_.type,
      sub_desc.level,
      sub_desc.x_offset, sub_desc.y_offset,
      sub_desc.width, sub_desc.height,
      desc_.internal_format,
      sub_desc.data_size,
      data);
  } else {
    glPixelStorei(GL_UNPACK_ALIGNMENT, sub_desc.alignment);
    glTexSubImage2D(desc_.type,
      sub_desc.level,
      sub_desc.x_offset, sub_desc.y_offset,
      sub_desc.width, sub_desc.height,
      desc_.format,
      desc_.data_type,
      data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);
  }

  glBindTexture(desc_.type, 0);
  return true;
}


///////////////////////////////////////////////////////

Texture3D::Texture3D() {
}

Texture3D::~Texture3D() {
}

bool Texture3D::Create(const TextureDesc &desc) {
  if (desc.type != GL_TEXTURE_3D)
    return false;

  if (desc.compressed && !desc.src_data)
    return false;

  desc_ = desc;
  glBindTexture(desc_.type, handle_);

  int actual_levels = desc_.levels;
  if (actual_levels == 0) {  // manually calculate max level
    actual_levels = (int)std::floor(std::log2(
      (float)std::max(std::max(desc_.width, desc_.height), desc_.depth))) + 1;
  }

  //setup sampling parameters
  glTexParameteri(desc_.type, GL_TEXTURE_MAG_FILTER, gl::MapFilterMode(desc_.mag_filter));
  glTexParameteri(desc_.type, GL_TEXTURE_MIN_FILTER, gl::MapFilterMode(desc_.min_filter));
  glTexParameteri(desc_.type, GL_TEXTURE_WRAP_S, gl::MapWrapMode(desc_.wrap_s));
  glTexParameteri(desc_.type, GL_TEXTURE_WRAP_T, gl::MapWrapMode(desc_.wrap_t));
  glTexParameteri(desc_.type, GL_TEXTURE_MAX_LEVEL, actual_levels - 1);
  glTexParameteri(desc_.type, GL_TEXTURE_COMPARE_MODE,
    desc_.depth_compare ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
  glTexParameteri(desc_.type, GL_TEXTURE_COMPARE_FUNC, desc_.depth_func);

  bool initialize = (bool)desc_.src_data; //is initial data provided?
  if (desc_.levels == 0) {  //automatically generate mipmaps
    if (initialize) {
      glm::ivec3 dims = desc_.src_data->Dimensions(0);
      const void *data = desc_.src_data->Data(0, 0, 0);
      if (desc_.compressed) {
        uint32_t data_size = desc_.src_data->DataSize(0);
        glCompressedTexImage3D(desc_.type, 0, desc_.internal_format,
          dims[0], dims[1], dims[2], 0, data_size, data);
      } else {
        int alignment = desc_.src_data->Alignment(0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        glTexImage3D(desc_.type, 0, desc_.internal_format,
          dims[0], dims[1], dims[2], 0, desc_.format,
          desc_.data_type, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);
      }
    } else {
      glTexImage3D(desc_.type, 0, desc_.internal_format,
        desc_.width, desc_.height, desc_.depth, 0, desc_.format,
        desc_.data_type, 0);
    }
    glGenerateMipmap(desc_.type);
  } else {  //precomputed mipmaps
    for (uint32_t level = 0; level < desc_.levels; ++level) {
      if (initialize) {
        glm::ivec3 dims = desc_.src_data->Dimensions(level);
        const void *data = desc_.src_data->Data(0, 0, level);
        if (desc_.compressed) {
          uint32_t data_size = desc_.src_data->DataSize(level);
          glCompressedTexImage3D(desc_.type, level, desc_.internal_format,
            dims[0], dims[1], dims[2], 0, data_size, data);
        } else {
          int alignment = desc_.src_data->Alignment(level);
          glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
          glTexImage3D(desc_.type, level, desc_.internal_format,
            dims[0], dims[1], dims[2], 0, desc_.format,
            desc_.data_type, data);
        }
      } else {
        uint32_t width = desc_.width >> level; uint32_t height = desc_.height >> level;
        uint32_t depth = desc_.depth >> level;
        glTexImage3D(desc_.type, level, desc_.internal_format,
          width, height, depth, 0, desc_.format,
          desc_.data_type, 0);
      }
    }
    if (initialize) {
      glPixelStorei(GL_UNPACK_ALIGNMENT, kDefaultAlignment);
    }
  }
  glBindTexture(desc_.type, 0);
  desc_.src_data.reset();
  CHKGLERR_RET

  return true;
}

bool Texture3D::UpdateTexture(const SubTextureDesc &sub_desc, const void *data) {
  return false;
}

}
