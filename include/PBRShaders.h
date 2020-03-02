#ifndef MINEOLA_PBRSHADERS_H
#define MINEOLA_PBRSHADERS_H

#include <string>

namespace mineola {

struct MaterialFlags {
  void EnableDiffuseMap();
  void EnableOcclusionMap();
  void EnableNormalMap();
  void EnableMetallicRoughnessMap();
  void EnableEmissiveMap();
  void EnableBlending();
  void EnableAlphaCutOff();
  void Clear();

  bool HasDiffuseMap() const;
  bool HasOcclusionMap() const;
  bool HasNormalMap() const;
  bool HasMetallicRoughnessMap() const;
  bool HasEmissiveMap() const;
  bool IsBlendingEnabled() const;
  bool IsAlphaCutOffEnabled() const;
  bool HasTextures() const;

  std::string Abbrev() const;

  uint8_t flags{0};

  enum {
    DIFFUSE_MAP_BIT = 0x1, OCCLUSION_MAP_BIT = 0x2, NORMAL_MAP_BIT = 0x4,
    METALLIC_ROUGHNESS_MAP_BIT = 0x8, EMISSIVE_MAP_BIT = 0x10,
    ALPHA_BLEND_BIT = 0x20, ALPHA_CUTOFF_BIT = 0x40
  };
};

struct AttribFlags {
  void EnableNormal();
  void EnableTangent();
  void EnableTexCoord();
  void EnableColor();
  void EnableSkinning();
  void Clear();

  bool HasNormal() const;
  bool HasTangent() const;
  bool HasTexCoord() const;
  bool HasColor() const;
  bool HasSkin() const;

  std::string Abbrev() const;

  uint8_t flags{0};

  enum {
    NORMAL_BIT = 0x1, TANGENT_BIT = 0x2, TEXCOORD_BIT = 0x4, COLOR_BIT = 0x8,
    SKIN_BIT = 0x10
  };
};

std::string SelectOrCreatePBREffect(bool srgb,
  const MaterialFlags &mat_flags, const AttribFlags &attrib_flags);

} //end namespace

#endif
