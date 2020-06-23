#include "prefix.h"
#include <mineola/glutility.h>
#include <mineola/AppHelper.h>
#include <mineola/Engine.h>
#include <mineola/GLEffect.h>
#include <mineola/Material.h>

namespace mineola {

bool LoadBuiltInShaders() {
  Engine &en = Engine::Instance();

  std::vector<std::unique_ptr<RenderState>> render_states;

  // load default shaders
  const char defaultvs[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "in vec3 Pos;\n"
  "void main(void)\n"
  "{\n"
  "    vec4 pos = _model_mat * vec4(Pos, 1.0);\n"
  "    gl_Position = _proj_view_mat * pos;\n"
  "}";

  const char defaultps[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "out vec4 fragColor;\n"
  "void main(void)\n"
  "{\n"
  "   fragColor = vec4(1, 0, 1, 1);\n"
  "}";

  if (!CreateEffectFromMemHelper("mineola:effect:fallback",
    defaultvs, defaultps, nullptr, std::move(render_states))) {
    return false;
  }

  const char diffusevs[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "in vec3 Pos;\n"
  "in vec3 Normal;\n"
  "out vec3 normal_wc;\n"
  "void main(void)\n"
  "{\n"
  "    vec4 pos = _model_mat * vec4(Pos, 1.0);\n"
  "    normal_wc = mat3(transpose(inverse(_model_mat))) * Normal;\n"
  "    gl_Position = _proj_view_mat * pos;\n"
  "}";

  const char diffuseps[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "in vec3 normal_wc;\n"
  "out vec4 fragColor;\n"
  "void main(void)\n"
  "{\n"
  "    float light = max(0.0, dot(normalize(normal_wc), normalize(_light_pos_0.xyz)));\n"
  "    fragColor = vec4(light, light, light, 1);\n"
  "}";

  render_states.clear();
  if (!CreateEffectFromMemHelper("mineola:effect:diffuse",
    diffusevs, diffuseps, nullptr, std::move(render_states))) {
    return false;
  }

  const char diffuse_tex_vs[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "in vec3 Pos;\n"
  "in vec3 Normal;\n"
  "in vec2 TexCoord0;\n"
  "out vec3 normal_wc;\n"
  "out vec2 texcoord;\n"
  "void main(void)\n"
  "{\n"
  "    vec4 pos = _model_mat * vec4(Pos, 1.0);\n"
  "    normal_wc = mat3(transpose(inverse(_view_mat * _model_mat))) * Normal;\n"
  "    texcoord = TexCoord0;\n"
  "    gl_Position = _proj_view_mat * pos;\n"
  "}";

  const char diffuse_tex_ps[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "uniform sampler2D diffuse_sampler;\n"
  "in vec3 normal_wc;\n"
  "in vec2 texcoord;\n"
  "out vec4 fragColor;\n"
  "void main(void)\n"
  "{\n"
  "    float light = max(0.0, dot(normalize(normal_wc), normalize(_light_pos_0.xyz)));\n"
  "    vec4 color = texture(diffuse_sampler, texcoord);\n"
  "    fragColor = vec4(color.xyz * light, 1);\n"
  "}";

  render_states.clear();
  if (!CreateEffectFromMemHelper("mineola:effect:diffusetex",
    diffuse_tex_vs, diffuse_tex_ps, nullptr, std::move(render_states))) {
    return false;
  }

  const char ambient_tex_vs[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "in vec3 Pos;\n"
  "in vec2 TexCoord0;\n"
  "out vec2 texcoord;\n"
  "void main(void)\n"
  "{\n"
  "    vec4 pos = _model_mat * vec4(Pos, 1.0);\n"
  "    texcoord = TexCoord0;\n"
  "    gl_Position = _proj_view_mat * pos;\n"
  "}";

  const char ambient_tex_ps[] =
  "#version 300 es\n"
  "precision mediump float;\n"
  "#include \"mineola_builtin_uniforms\"\n"
  "uniform sampler2D diffuse_sampler;\n"
  "in vec2 texcoord;\n"
  "out vec4 fragColor;\n"
  "void main(void)\n"
  "{\n"
  "    vec4 color = texture(diffuse_sampler, texcoord);\n"
  "    fragColor = vec4(color.xyz, 1);\n"
  "}";

  render_states.clear();
  if (!CreateEffectFromMemHelper("mineola:effect:ambienttex",
    ambient_tex_vs, ambient_tex_ps, nullptr, std::move(render_states))) {
    return false;
  }

  return true;
}

bool InitEngine() {
  Engine &en = Engine::Instance();
  en.Init();

  if (!LoadBuiltInShaders()) {
    return false;
  }

  // create default material
  std::shared_ptr<Material> mat(new Material);
  mat->alpha = 1.0f;
  mat->specularity = 30.0f;
  mat->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  mat->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  mat->specular = glm::vec3(0.2f, 0.2f, 0.2f);
  mat->emit = glm::vec3(0.f, 0.f, 0.f);
  mat->texture_slots["diffuse_sampler"] = {"mineola:texture:fallback"};
  en.ResrcMgr().Add("mineola:material:fallback", bd_cast<Resource>(mat));

  // gl switches
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);

  return true;
}

void StartEngine() {
  auto &en = Engine::Instance();
  en.Start();
}

void ReleaseEngine() {
  auto &en = Engine::Instance();
  en.Release();
}

void SetScreenFramebuffer(int fbo, uint32_t w, uint32_t h) {
  auto &en = Engine::Instance();
  en.SetDefaultFramebuffer(fbo, w, h);
}

void ResizeScreen(uint32_t w, uint32_t h) {
  auto &en = Engine::Instance();
  en.OnSize(w, h);
}

void AddSearchPath(const char *path) {
  auto &en = Engine::Instance();
  en.ResrcMgr().AddSearchPath(path);
}

void FrameMove() {
  auto &en = Engine::Instance();
  en.FrameMove();
}

void Render() {
  auto &en = Engine::Instance();
  en.Render();
}

Engine &GetEngine() {
  return Engine::Instance();
}

}
