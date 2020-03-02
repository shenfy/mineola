#ifndef MINEOLA_TEXTURE_H
#define MINEOLA_TEXTURE_H

#include <cstdint>
#include <vector>
#include <boost/noncopyable.hpp>
#include "BasisObj.h"
#include "TextureDesc.h"

namespace mineola {

class Texture : public Resource, boost::noncopyable {
public:
  Texture();
  virtual ~Texture();
  virtual bool Create(const TextureDesc &desc) = 0;
  /**
   * Bind texture to the regular texture image unit
   */
  virtual void Bind();

  uint32_t Handle() const;
  const TextureDesc &Desc() const;

protected:
  uint32_t handle_;
  TextureDesc desc_;

};

class ExternalTexture : public Texture {
public:
  ExternalTexture(uint32_t handle, const TextureDesc &desc);
  virtual ~ExternalTexture();

  virtual bool Create(const TextureDesc &desc) override;
};

class InternalTexture : public Texture {
public:
  InternalTexture();
  virtual ~InternalTexture();
  virtual bool UpdateTexture(const SubTextureDesc &desc, const void *data) = 0;
};

class Texture2D : public InternalTexture {
public:
  Texture2D();
  virtual ~Texture2D();
  virtual bool Create(const TextureDesc &desc) override;
  virtual bool UpdateTexture(const SubTextureDesc &desc, const void *data) override;
};

class Texture3D : public InternalTexture {
public:
  Texture3D();
  virtual ~Texture3D();
  virtual bool Create(const TextureDesc &desc) override;
  virtual bool UpdateTexture(const SubTextureDesc &desc, const void *data) override;
};

} //namespace


#endif
