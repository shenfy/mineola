#include "prefix.h"
#include <mineola/PrefabHelper.h>
#include <mineola/SceneNode.h>
#include <mineola/GLEffect.h>
#include <mineola/PrimitiveHelper.h>
#include <mineola/Renderable.h>
#include <mineola/RenderState.h>

namespace {

auto skybox_vert = R"(#version 300 es
precision highp float;
in vec3 Pos;

void main() {
  gl_Position = vec4(Pos.xy, 1.0, 1.0);
})";

auto skybox_frag = R"(#version 300 es
precision highp float;
#include "mineola_builtin_uniforms"

out vec4 color;

const float PI = 3.1415926535897932;

#if defined(SRGB_ENCODE)
float L2SRGB(float v) {
  return v <= 0.0031308 ? 12.92 * v : (1.055 * pow(v, 0.4167) - 0.055);
}

vec4 SRGBEncode(vec4 rgba) {
  return vec4(L2SRGB(rgba.r), L2SRGB(rgba.g), L2SRGB(rgba.b), rgba.a);
}
#endif

void main() {
  vec2 vp = gl_FragCoord.xy / _viewport_size.xy;
  vec4 ndc = vec4(vp * 2.0 - 1.0, 1.0, 1.0);
  vec4 view = inverse(_proj_mat) * ndc;
  vec4 world = _view_mat_inv * vec4(view.xyz, 0.0);
  vec3 env = normalize(mat3(_env_light_mat_0) * world.xyz);
  float theta = acos(env.y);
  float phi = atan(env.x, env.z);
  vec2 uv = vec2((phi + PI) / PI * 0.5, theta / PI);
  color = textureLod(_env_light_probe_0, uv, 0.0);
  #if defined(SRGB_ENCODE)
  color = SRGBEncode(color);
  #endif
}
)";

}

namespace mineola::prefab_helper {

bool CreateSkybox(int layer_mask, bool srgb, SceneNode &node) {
  const char *effect_name = "mineola:effect:skybox";
  using namespace render_state;
  std::vector<std::unique_ptr<RenderState>> states;
  states.push_back(std::make_unique<DepthTestState>(true));
  states.push_back(std::make_unique<DepthFuncState>(kCmpFuncLEqual));
  states.push_back(std::make_unique<CullEnableState>(false));
  effect_defines_t macros;
  if (srgb) {
    macros.push_back({"SRGB_ENCODE", {}});
  }
  if (!CreateEffectFromMemHelper(effect_name, skybox_vert, skybox_frag,
    &macros, std::move(states))) {
    return false;
  }

  auto va = std::make_shared<vertex_type::VertexArray>();
  primitive_helper::BuildRectXY(2.0f, *va);
  auto renderable = std::make_shared<Renderable>();
  renderable->AddVertexArray(std::move(va), "mineola:material:fallback");
  renderable->SetEffect(effect_name);
  renderable->SetLayerMask(layer_mask);
  node.Renderables().push_back(std::move(renderable));
  return true;
}

void CreateAxes(int layer_mask, SceneNode &node) {
  auto va = std::make_shared<vertex_type::VertexArray>();
  primitive_helper::BuildFrameAxes(1.0f, *va);
  auto renderable = std::make_shared<Renderable>();
  renderable->AddVertexArray(std::move(va), "mineola:material:fallback");
  renderable->SetEffect("mineola:effect:diffusecolor");
  renderable->SetLayerMask(layer_mask);
  node.Renderables().push_back(std::move(renderable));
}

}
