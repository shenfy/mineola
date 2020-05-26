#ifndef MINEOLA_PBRSHADERS_H
#define MINEOLA_PBRSHADERS_H

#include <string>
#include <array>

namespace mineola {

struct MaterialFlags {
  void EnableDiffuseMap(int uv = 0);
  void EnableOcclusionMap(int uv = 0);
  void EnableNormalMap(int uv = 0);
  void EnableMetallicRoughnessMap(int uv = 0);
  void EnableEmissiveMap(int uv = 0);
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

  uint8_t flags {0};

  enum {kNumTextures = 5};
  std::array<uint8_t, kNumTextures> tex_uvs {0, 0, 0, 0, 0};

  enum {DIFFUSE_UV_LOC = 0, OCC_UV_LOC = 1, NORM_UV_LOC = 2,
    METAL_ROUGH_UV_LOC = 3, EMIT_UV_LOC = 4};

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
  void EnableTexCoord2();
  void EnableColor();
  void EnableSkinning();
  void Clear();

  bool HasNormal() const;
  bool HasTangent() const;
  bool HasTexCoord() const;
  bool HasTexCoord2() const;
  bool HasColor() const;
  bool HasSkin() const;

  std::string Abbrev() const;

  uint8_t flags{0};

  enum {
    NORMAL_BIT = 0x1, TANGENT_BIT = 0x2, TEXCOORD_BIT = 0x4, TEXCOORD2_BIT = 0x8,
    COLOR_BIT = 0x10, SKIN_BIT = 0x20
  };
};

std::string SelectOrCreatePBREffect(bool srgb,
  const MaterialFlags &mat_flags, const AttribFlags &attrib_flags);

} //end namespace

#endif
