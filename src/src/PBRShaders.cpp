#include "prefix.h"
#include <mineola/PBRShaders.h>
#include <vector>
#include <memory>
#include <mineola/Engine.h>
#include <mineola/GLEffect.h>
#include <mineola/RenderState.h>

namespace {

using namespace mineola;

const char pbr_vs_str[] =
R"(#version 300 es
precision highp float;
#include "mineola_builtin_uniforms"

in vec3 Pos;
out vec3 pos_wc;

#if defined(HAS_NORMAL)
in vec3 Normal;
out vec3 normal;
#endif  // HAS_NORMAL
#if defined(HAS_TANGENT)
in vec4 Tangent;
out mat3 tbn;
#endif  // HAS_TANGENT
#if defined(HAS_TEXCOORD)
in vec2 TexCoord0;
out vec2 texcoord0;
#endif  // HAS_TEXCOORD
#if defined(HAS_TEXCOORD2)
in vec2 TexCoord1;
out vec2 texcoord1;
#endif  // HAS_TEXCOORD2
#if defined(HAS_COLOR)
in vec3 Diffuse;
out vec3 vcolor;
#endif  // HAS_COLOR

#if defined(HAS_SKIN)
#include "mineola_skinned_animation"
#endif

vec3 Dir2WC(mat4 model_mat, vec3 dir) {
  return normalize((model_mat * vec4(normalize(dir), 0.0)).xyz);
}

void main(void) {
  #if defined(HAS_SKIN)
  mat4 model_mat = BlendWeight.x * _joint_mats[int(BlendIdx.x)]
    + BlendWeight.y * _joint_mats[int(BlendIdx.y)]
    + BlendWeight.z * _joint_mats[int(BlendIdx.z)]
    + BlendWeight.w * _joint_mats[int(BlendIdx.w)];
  #else
  mat4 model_mat = _model_mat;
  #endif
  vec4 pos = model_mat * vec4(Pos, 1.0);
  pos_wc = pos.xyz / pos.w;
  gl_Position = _proj_view_mat * pos;

  #if defined(HAS_NORMAL)
  normal = Dir2WC(model_mat, Normal);
  #endif
  #if defined(HAS_TEXCOORD)
  texcoord0 = TexCoord0;
  #endif
  #if defined(HAS_TEXCOORD2)
  texcoord1 = TexCoord1;
  #endif
  #if defined(HAS_TANGENT)
  vec3 tangent = Dir2WC(model_mat, Tangent.xyz);
  vec3 bitangent = normalize(cross(normal, tangent) * Tangent.w);
  tbn = mat3(tangent, bitangent, normal);
  #endif
  #if defined(HAS_COLOR)
  vcolor = Diffuse;
  #endif
})";

const char pbr_fs_str[] =
R"(#version 300 es
precision highp float;
#include "mineola_builtin_uniforms"

#if defined(HAS_ALBEDO_MAP)
uniform sampler2D diffuse_sampler;  // base color
#endif
#if defined(HAS_AO_MAP)
uniform sampler2D lightmap_sampler;  // occlusion map
#endif
#if defined(HAS_NORMAL_MAP)
uniform sampler2D normal_sampler;  // normal map
#endif
#if defined(HAS_METALLIC_MAP)
uniform sampler2D metallic_roughness_sampler;  // metallic roughness
#endif
#if defined(HAS_EMISSIVE_MAP)
uniform sampler2D emissive_sampler;  // emission map
#endif
#if defined(USE_ALPHA_MASK)
uniform float alpha_threshold;  // alpha cut-off threshold
#endif

uniform vec3 diffuse;  // albedo coefficient
uniform float alpha;  // albedo transparency
uniform float specularity;  // metallic coefficient
uniform float roughness;  // roughness coefficient
uniform vec3 emit;  // emissive coefficient

in vec3 pos_wc;

#if defined(HAS_NORMAL)
in vec3 normal;
#endif
#if defined(HAS_TEXCOORD)
in vec2 texcoord0;
#endif
#if defined(HAS_TEXCOORD2)
in vec2 texcoord1;
#endif
#if defined(HAS_TANGENT)
in mat3 tbn;
#endif
#if defined(HAS_COLOR)
in vec3 vcolor;
#endif

out vec4 frag_color;

const float dielectric_specular = 0.04;
const vec3 black = vec3(0.0, 0.0, 0.0);
const float PI = 3.1415926535897932;

float pow2(float x) {
  return x * x;
}
vec3 SpecularColor(vec3 albedo, float metallic) {
  return mix(vec3(dielectric_specular), albedo, metallic);
}
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 SchlickFresnelFast(vec3 f0, float v_dot_h) {
  float approx = exp2((-5.55473 * v_dot_h - 6.98316) * v_dot_h);
  return f0 + (1.0 - f0) * approx;
}
vec3 DiffuseTerm(vec3 albedo, float n_dot_l, float metallic) {
  vec3 c_diff = mix(albedo * 0.96, vec3(0.0), metallic);
  return c_diff * n_dot_l / PI;
}
float D_GGX(float n_dot_h, float a2) {
  float factor = pow2(n_dot_h) * (a2 - 1.0) + 1.0;
  return a2 / pow2(factor) / PI;
}
float G_GGX_UE4(float n_dot_l, float n_dot_v, float roughness) {
  float k = pow2(roughness + 1.0) / 8.0;
  float gl = n_dot_l / (n_dot_l * (1.0 - k) + k);
  float gv = n_dot_v / (n_dot_v * (1.0 - k) + k);
  return gl * gv;
}
vec3 SpecularTerm(vec3 color_specular,
  float n_dot_h, float n_dot_l, float n_dot_v, float v_dot_h,
  float roughness, float a2) {
  vec3 F = SchlickFresnelFast(color_specular, v_dot_h);
  float D = D_GGX(n_dot_h, a2);
  float G = G_GGX_UE4(n_dot_l, n_dot_v, roughness);
  return D * G * F / (4.0 * n_dot_l * n_dot_v);
}
void CalcDotProducts(vec3 light_dir, vec3 view_dir, vec3 normal_dir,
  out vec3 half_dir,
  out float n_dot_l, out float n_dot_v, out float n_dot_h, out float v_dot_h) {
  half_dir = normalize(light_dir + view_dir);
  n_dot_l = clamp(dot(normal_dir, light_dir), 0.0, 1.0);
  n_dot_v = clamp(dot(normal_dir, view_dir), 0.0, 1.0);
  n_dot_h = clamp(dot(normal_dir, half_dir), 0.0, 1.0);
  v_dot_h = clamp(dot(view_dir, half_dir), 0.0, 1.0);
}
vec3 EnvlightDiffuseTerm(vec3 albedo, vec3 normal_wc, float metallic) {
  vec3 n = normalize(mat3(_env_light_mat_0) * normal_wc);
  vec4 e = _env_light_sh3_0[0] * 0.886227;
  e += _env_light_sh3_0[1] * (2.0 * 0.511664 * n.y); // ( 2 * π / 3 ) * 0.488603
  e += _env_light_sh3_0[2] * (2.0 * 0.511664 * n.z);
  e += _env_light_sh3_0[3] * (2.0 * 0.511664 * n.x);
  e += _env_light_sh3_0[4] * (2.0 * 0.429043 * n.x * n.y); // ( π / 4 ) * 1.092548
  e += _env_light_sh3_0[5] * (2.0 * 0.429043 * n.y * n.z);
  e += _env_light_sh3_0[6] * (0.743125 * n.z * n.z - 0.247708); // ( π / 4 ) * 0.315392 * 3
  e += _env_light_sh3_0[7] * (2.0 * 0.429043 * n.x * n.z);
  e += _env_light_sh3_0[8] * (0.429043 * (n.x * n.x - n.y * n.y)); // ( π / 4 ) * 0.546274
  vec3 c_diff = mix(albedo * 0.96, vec3(0.0), metallic);
  return c_diff * e.rgb / PI;
}
vec3 EnvLightSpecularTerm(vec3 color_specular, vec3 normal_wc, vec3 view_wc, float roughness) {
  float n_dot_v = dot(normal_wc, view_wc);
  vec2 ab = texture(_env_brdf, vec2(n_dot_v, roughness)).rg;
  vec3 brdf = color_specular * ab.x + ab.y;
  // Point lobe in off-specular peak direction
  vec3 r0 = 2.0 * n_dot_v * normal_wc - view_wc;
  float a = pow2(roughness);
  vec3 r = normalize(mix(normal_wc, r0, (1.0 - a) * (sqrt(1.0 - a) + a)));
  r = normalize(mat3(_env_light_mat_0) * r);
  float theta = acos(r.y);
  float phi = atan(r.x, r.z);
  ivec2 tex_size = textureSize(_env_light_probe_0, 0);
  float mip_count = log2(float(max(tex_size.x, tex_size.y))) + 1.0;
  float level = (mip_count - 1.0) * roughness;
  vec3 light = textureLod(_env_light_probe_0, vec2((phi + PI) / PI * 0.5, theta / PI), level).rgb;
  return light * brdf;
}

#if defined(SRGB_ENCODE)
float L2SRGB(float v) {
  return v <= 0.0031308 ? 12.92 * v : (1.055 * pow(v, 0.4167) - 0.055);
}
vec4 SRGBEncode(vec4 rgba) {
  return vec4(L2SRGB(rgba.r), L2SRGB(rgba.g), L2SRGB(rgba.b), rgba.a);
}
#endif

void main(void) {
  #if defined(HAS_COLOR)
  vec4 base_color = vec4(vcolor, 1.0);
  #else
  vec4 base_color = vec4(1.0);
  #endif

  #if defined(HAS_ALBEDO_MAP) && defined(DIFFUSE_TEXCOORD)
  base_color *= texture(diffuse_sampler, DIFFUSE_TEXCOORD) * vec4(diffuse, alpha);
  #else
  base_color *= vec4(diffuse, alpha);
  #endif

  #if defined(USE_ALPHA_MASK)
  if (base_color.a < alpha_threshold) {
    discard;
  }
  #endif

  #if defined(IS_UNLIT)
  frag_color = base_color;
  return;
  #endif

  #if defined(HAS_AO_MAP) && defined(AO_TEXCOORD)
  float ao = texture(lightmap_sampler, AO_TEXCOORD).x;
  #else
  float ao = 1.0;
  #endif

  #if defined(HAS_METALLIC_MAP) && defined(METAL_TEXCOORD)
  vec4 o_m_r = texture(metallic_roughness_sampler, METAL_TEXCOORD);
  float rough = o_m_r.y;
  float metallic = o_m_r.z;
  #else
  float rough = roughness;
  float metallic = specularity;
  #endif

  #if defined(HAS_EMISSIVE_MAP) && defined(EMIT_TEXCOORD)
  vec3 emission = texture(emissive_sampler, EMIT_TEXCOORD).rgb * emit;
  #else
  vec3 emission = emit;
  #endif

  vec4 eye_wc = _view_mat_inv * vec4(0.0, 0.0, 0.0, 1.0);
  eye_wc /= eye_wc.w;
  vec3 light_wc = _light_pos_0.xyz;
  // vec3 light_dir = normalize(light_wc - pos_wc);
  vec3 light_dir = normalize(light_wc);
  vec3 view_dir = normalize(eye_wc.xyz - pos_wc);

  #if defined(HAS_NORMAL_MAP) && defined(HAS_TANGENT) && defined(NORMAL_TEXCOORD)
  vec3 normal_pp = texture(normal_sampler, NORMAL_TEXCOORD).xyz;
  normal_pp = normalize(normal_pp * 2.0 - 1.0);
  vec3 normal_dir = normalize(tbn * normal_pp);
  #elif defined(HAS_NORMAL)
  vec3 normal_dir = normalize(normal);
  #else
  vec3 normal_dir = vec3(0.0, 0.0, 0.0);
  frag_color = vec4(base_color.rgb * ao, base_color.a);
  return;
  #endif

  #if defined(DOUBLE_SIDE)
  if (!gl_FrontFacing) {
    normal_dir = -normal_dir;
  }
  #endif

  vec3 half_dir;
  float n_dot_l;
  float n_dot_v;
  float n_dot_h;
  float v_dot_h;
  CalcDotProducts(light_dir, view_dir, normal_dir,
    half_dir, n_dot_l, n_dot_v, n_dot_h, v_dot_h);
  vec3 albedo = base_color.rgb;
  vec3 diffuse_term = DiffuseTerm(albedo, n_dot_l, metallic);
  float a = pow2(rough);
  float a2 = pow2(a);
  vec3 color_specular = SpecularColor(albedo, metallic);
  vec3 specular_term = SpecularTerm(color_specular, n_dot_h, n_dot_l, n_dot_v, v_dot_h, rough, a2);
  vec3 color_result = emission
    + diffuse_term * _light_intensity_0.rgb * ao
    + max(specular_term * n_dot_l * _light_intensity_0.rgb, vec3(0.0));

  #if defined(USE_ENV_LIGHT)
  color_result += EnvlightDiffuseTerm(albedo, normal_dir, metallic) * ao;
  color_result += EnvLightSpecularTerm(color_specular, normal_dir, view_dir, rough) * ao;
  #else
  color_result += albedo * 0.1;
  #endif

  #if defined(SRGB_ENCODE)
  frag_color = SRGBEncode(vec4(color_result, base_color.a));
  #else
  frag_color = vec4(color_result, base_color.a);
  #endif
})";

std::string BuildTCName(uint8_t idx) {
  std::string result("texcoord0");
  result[8] = idx + '0';
  return result;
}

effect_defines_t CreatePBRShaderMacros(
  const MaterialFlags &mat_flags, const AttribFlags &attrib_flags) {
  effect_defines_t result;
  if (attrib_flags.HasNormal()) {
    result.push_back({"HAS_NORMAL", {}});
  }
  if (attrib_flags.HasTangent()) {
    result.push_back({"HAS_TANGENT", {}});
  }
  if (attrib_flags.HasTexCoord()) {
    result.push_back({"HAS_TEXCOORD", {}});
  }
  if (attrib_flags.HasTexCoord2()) {
    result.push_back({"HAS_TEXCOORD2", {}});
  }
  if (attrib_flags.HasColor()) {
    result.push_back({"HAS_COLOR", {}});
  }
  if (attrib_flags.HasSkin()) {
    result.push_back({"HAS_SKIN", {}});
  }

  if (mat_flags.HasDiffuseMap()) {
    result.push_back({"HAS_ALBEDO_MAP", {}});
    result.push_back({"DIFFUSE_TEXCOORD",
      BuildTCName(mat_flags.tex_uvs[MaterialFlags::DIFFUSE_UV_LOC])});
  }
  if (mat_flags.IsAlphaCutOffEnabled()) {
    result.push_back({"USE_ALPHA_MASK", {}});
  }
  if (mat_flags.IsUnlit()) {
    result.push_back({"IS_UNLIT", {}});
  } else {
    if (mat_flags.HasOcclusionMap()) {
      result.push_back({"HAS_AO_MAP", {}});
      result.push_back({"AO_TEXCOORD",
        BuildTCName(mat_flags.tex_uvs[MaterialFlags::OCC_UV_LOC])});
    }
    if (mat_flags.HasNormalMap()) {
      result.push_back({"HAS_NORMAL_MAP", {}});
      result.push_back({"NORMAL_TEXCOORD",
        BuildTCName(mat_flags.tex_uvs[MaterialFlags::NORM_UV_LOC])});
    }
    if (mat_flags.HasMetallicRoughnessMap()) {
      result.push_back({"HAS_METALLIC_MAP", {}});
      result.push_back({"METAL_TEXCOORD",
        BuildTCName(mat_flags.tex_uvs[MaterialFlags::METAL_ROUGH_UV_LOC])});
    }
    if (mat_flags.HasEmissiveMap()) {
      result.push_back({"HAS_EMISSIVE_MAP", {}});
      result.push_back({"EMIT_TEXCOORD",
        BuildTCName(mat_flags.tex_uvs[MaterialFlags::EMIT_UV_LOC])});
    }
  }
  if (mat_flags.IsDoubleSided()) {
    result.push_back({"DOUBLE_SIDE", {}});
  }
  return result;
}

}  // namespace


namespace mineola {

void MaterialFlags::EnableDiffuseMap(int uv) {
  flags |= DIFFUSE_MAP_BIT;
  tex_uvs[DIFFUSE_UV_LOC] = uv;
}

void MaterialFlags::EnableOcclusionMap(int uv) {
  flags |= OCCLUSION_MAP_BIT;
  tex_uvs[OCC_UV_LOC] = uv;
}

void MaterialFlags::EnableNormalMap(int uv) {
  flags |= NORMAL_MAP_BIT;
  tex_uvs[NORM_UV_LOC] = uv;
}

void MaterialFlags::EnableMetallicRoughnessMap(int uv) {
  flags |= METALLIC_ROUGHNESS_MAP_BIT;
  tex_uvs[METAL_ROUGH_UV_LOC] = uv;
}

void MaterialFlags::EnableEmissiveMap(int uv) {
  flags |= EMISSIVE_MAP_BIT;
  tex_uvs[EMIT_UV_LOC] = uv;
}

void MaterialFlags::EnableBlending() {
  flags |= ALPHA_BLEND_BIT;
}

void MaterialFlags::EnableAlphaCutOff() {
  flags |= ALPHA_CUTOFF_BIT;
}

void MaterialFlags::SetUnlit() {
  flags |= UNLIT_BIT;
}

void MaterialFlags::EnableDoubleSide() {
  flags |= DOUBLE_SIDE_BIT;
}

void MaterialFlags::Clear() {
  flags = 0;
}

bool MaterialFlags::HasDiffuseMap() const {
  return flags & DIFFUSE_MAP_BIT;
}

bool MaterialFlags::HasOcclusionMap() const {
  return flags & OCCLUSION_MAP_BIT;
}

bool MaterialFlags::HasNormalMap() const {
  return flags & NORMAL_MAP_BIT;
}

bool MaterialFlags::HasMetallicRoughnessMap() const {
  return flags & METALLIC_ROUGHNESS_MAP_BIT;
}

bool MaterialFlags::HasEmissiveMap() const {
  return flags & EMISSIVE_MAP_BIT;
}

bool MaterialFlags::IsBlendingEnabled() const {
  return flags & ALPHA_BLEND_BIT;
}

bool MaterialFlags::IsAlphaCutOffEnabled() const {
  return flags & ALPHA_CUTOFF_BIT;
}

bool MaterialFlags::HasTextures() const {
  return flags & (
    DIFFUSE_MAP_BIT | OCCLUSION_MAP_BIT | NORMAL_MAP_BIT | METALLIC_ROUGHNESS_MAP_BIT
    | EMISSIVE_MAP_BIT);
}

bool MaterialFlags::IsUnlit() const {
  return flags & UNLIT_BIT;
}

bool MaterialFlags::IsDoubleSided() const {
  return flags & DOUBLE_SIDE_BIT;
}

std::string MaterialFlags::Abbrev() const {
  std::string result = "d0o0n0m0e0aud";
  if (HasDiffuseMap()) {
    result[0] = 'D';
    result[1] = tex_uvs[DIFFUSE_UV_LOC] + '0';
  }
  if (HasOcclusionMap()) {
    result[2] = 'O';
    result[3] = tex_uvs[OCC_UV_LOC] + '0';
  }
  if (HasNormalMap()) {
    result[4] = 'N';
    result[5] = tex_uvs[NORM_UV_LOC] + '0';
  }
  if (HasMetallicRoughnessMap()) {
    result[6] = 'M';
    result[7] = tex_uvs[METAL_ROUGH_UV_LOC] + '0';
  }
  if (HasEmissiveMap()) {
    result[8] = 'E';
    result[9] = tex_uvs[EMIT_UV_LOC] + '0';
  }
  if (IsBlendingEnabled()) {
    result[10] = 'A';
  } else if (IsAlphaCutOffEnabled()) {
    result[10] = 'C';
  }
  if (IsUnlit()) {
    result[11] = 'U';
  }
  if (IsDoubleSided()) {
    result[12] = 'D';
  }
  return result;
}

void AttribFlags::EnableNormal() {
  flags |= NORMAL_BIT;
}

void AttribFlags::EnableTangent() {
  flags |= TANGENT_BIT;
}

void AttribFlags::EnableTexCoord() {
  flags |= TEXCOORD_BIT;
}

void AttribFlags::EnableTexCoord2() {
  flags |= TEXCOORD2_BIT;
}

void AttribFlags::EnableColor() {
  flags |= COLOR_BIT;
}

void AttribFlags::EnableSkinning() {
  flags |= SKIN_BIT;
}

void AttribFlags::Clear() {
  flags = 0;
}

bool AttribFlags::HasNormal() const {
  return flags & NORMAL_BIT;
}

bool AttribFlags::HasTangent() const {
  return flags & TANGENT_BIT;
}

bool AttribFlags::HasTexCoord() const {
  return flags & TEXCOORD_BIT;
}

bool AttribFlags::HasTexCoord2() const {
  return flags & TEXCOORD2_BIT;
}

bool AttribFlags::HasColor() const {
  return flags & COLOR_BIT;
}

bool AttribFlags::HasSkin() const {
  return flags & SKIN_BIT;
}

std::string AttribFlags::Abbrev() const {
  std::string result = "ntttcs";
  if (HasNormal()) {
    result[0] = 'N';
  }
  if (HasTangent()) {
    result[1] = 'T';
  }
  if (HasTexCoord()) {
    result[2] = 'T';
  }
  if (HasTexCoord2()) {
    result[3] = 'T';
  }
  if (HasColor()) {
    result[4] = 'C';
  }
  if (HasSkin()) {
    result[5] = 'S';
  }
  return result;
}

std::string SelectOrCreatePBREffect(bool srgb,
  const MaterialFlags &mat_flags, const AttribFlags &attrib_flags, bool use_env_light) {

  char srgb_abbre = srgb ? 'S' : 's';
  char env_light_abbre = use_env_light ? 'E' : 'e';

  std::string effect_name = "mineola:effect:pbr:"
    + attrib_flags.Abbrev()
    + mat_flags.Abbrev()
    + srgb_abbre
    + env_light_abbre;

  auto &en = Engine::Instance();
  auto effect = bd_cast<GLEffect>(en.ResrcMgr().Find(effect_name));
  if (!effect) {
    auto macros = CreatePBRShaderMacros(mat_flags, attrib_flags);
    if (srgb) {
      macros.push_back({"SRGB_ENCODE", {}});
    }
    if (use_env_light) {
      macros.push_back({"USE_ENV_LIGHT", {}});
    }

    std::vector<std::unique_ptr<RenderState>> render_states;
    render_states.push_back(std::make_unique<DepthTestState>(true));
    render_states.push_back(std::make_unique<DepthFuncState>(render_state::kCmpFuncLess));
    if (mat_flags.IsDoubleSided()) {
      render_states.push_back(std::make_unique<CullEnableState>(false));
    } else {
      render_states.push_back(std::make_unique<CullEnableState>(true));
      render_states.push_back(std::make_unique<CullFaceState>(render_state::kCullFaceBack));
    }

    if (mat_flags.IsBlendingEnabled()) {
      render_states.push_back(std::make_unique<BlendEnableState>(true));
      render_states.push_back(std::make_unique<BlendFuncState>(
        render_state::kBlendSrcAlpha, render_state::kBlendOneMinusSrcAlpha));
    } else {
      render_states.push_back(std::make_unique<BlendEnableState>(false));
    }

    if (!CreateEffectFromMemHelper(effect_name.c_str(),
      pbr_vs_str, pbr_fs_str, &macros, std::move(render_states))) {
      return {};
    }
  }

  return effect_name;
}

}  // namespace
