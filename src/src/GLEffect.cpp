#include "prefix.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <unordered_set>
#include <mineola/glutility.h>
#include <mineola/GLShader.h>
#include <mineola/GLEffect.h>
#include <mineola/VertexType.h>
#include <mineola/UniformBlock.h>
#include <mineola/UniformHelper.h>
#include <mineola/Engine.h>
#include <mineola/RenderState.h>

namespace mineola {

GLEffect::GLEffect() {}

GLEffect::~GLEffect() {
  vertex_shader_.reset();
  pixel_shader_.reset();
}

bool GLEffect::AttachShaders(std::shared_ptr<GLShader> vs,
	std::shared_ptr<GLShader> ps) {
  if (handle_ == 0) {
    MLOG("Cannot attach shaders to an invalid GLEffect!\n");
    return false;
  }

  if (vertex_shader_)
    glDetachShader(handle_, vertex_shader_->Handle());
  if (pixel_shader_)
    glDetachShader(handle_, pixel_shader_->Handle());

  glAttachShader(handle_, vs->Handle());
  glAttachShader(handle_, ps->Handle());
  vertex_shader_ = std::move(vs);
  pixel_shader_ = std::move(ps);

  glLinkProgram(handle_);
  if (!InfoLog()) return false;
  // reorder attributes
  ReorderAttribBindings();

  // reorder uniform blocks
  GenerateUniformBlockMap();
  BindBuiltinUniformBlocks();

  glLinkProgram(handle_);
  if (!InfoLog()) return false;

  if (!GenerateAttribMap()) return false;
  if (!GenerateVarMap()) return false;
  if (!GenerateUniformBlockMap()) return false;
  return true;
}

void GLEffect::ReorderAttribBindings() {
  int32_t attrib_num = 0;
  glGetProgramiv(handle_, GL_ACTIVE_ATTRIBUTES, &attrib_num);
  if (attrib_num <= 0)
    return;

  std::unordered_set<std::string> attrib_names;

  static char attrib_name[VAR_NAME_MAX_LENGTH];
  int32_t attrib_name_len, attrib_size;
  GLenum attrib_type;

  for (int i = 0; i < attrib_num; ++i) {
    glGetActiveAttrib(handle_, i, VAR_NAME_MAX_LENGTH,
              &attrib_name_len, &attrib_size, &attrib_type,
              attrib_name);
    attrib_names.insert(attrib_name);
  }

  for (int semantics = vertex_type::POSITION;
    semantics < vertex_type::RESERVED_SEMANTICS_NUM;
    ++semantics) {

    const char *var_name = vertex_type::GetSemanticsString(semantics);
    if (var_name == nullptr) {
      continue;
    }

    int bind_loc = vertex_type::GetSemanticsBindLocation(semantics);
    auto iter = attrib_names.find(var_name);
    if (iter != attrib_names.end()) {
      glBindAttribLocation(handle_, bind_loc, iter->c_str());
    }
  }
}

bool GLEffect::GenerateAttribMap() {
  if (handle_ == 0) {
    MLOG("Unable to generate attribute map for an invalid GLEffect!\n");
    return false;
  }

  int32_t attrib_num = 0;
  glGetProgramiv(handle_, GL_ACTIVE_ATTRIBUTES, &attrib_num);
  if (attrib_num < 0)
    return false;

  static char attrib_name[VAR_NAME_MAX_LENGTH];
  int32_t attrib_name_len, attrib_size, attrib_loc;
  GLenum attrib_type;
  for (int i = 0; i < attrib_num; ++i) {
    glGetActiveAttrib(handle_, i, VAR_NAME_MAX_LENGTH,
              &attrib_name_len, &attrib_size, &attrib_type,
              attrib_name);
    attrib_loc = glGetAttribLocation(handle_, attrib_name);
    if (attrib_loc != -1)
      attribute_map_[attrib_name] =
        std::make_tuple(attrib_loc, (uint32_t)attrib_type, attrib_size);
  }

  // dump attributes
  // for (auto iter = attribute_map_.begin(); iter != attribute_map_.end(); ++iter)
  //  printf("%s: %d, %d, %d\n", iter->first.c_str(), std::get<0>(iter->second), std::get<1>(iter->second), std::get<2>(iter->second));

  return true;
}

bool GLEffect::FindAttribLoc(const char *semantics,
  uint32_t format, uint32_t length, uint32_t *loc) const {
  auto iter = attribute_map_.find(semantics);
  if (iter != attribute_map_.end()) {  //found
    uint32_t type = 0, size = 0;
    type_mapping::MapGLType(std::get<1>(iter->second), &type, &size);
    if (type != format
      || type_mapping::SizeOf(type) * size != type_mapping::SizeOf(format) * length) {
      return false;
    }
    *loc = std::get<0>(iter->second);
    return true;
  }
  return false;
}

void GLEffect::SetRenderStates(std::vector<std::unique_ptr<RenderState>> states) {
  render_states_ = std::move(states);
}

void GLEffect::ApplyRenderStates() const {
  auto &mgr = Engine::Instance().RenderStateMgr();
  for (const auto &state : render_states_) {
    state->Apply(mgr);
  }
}

bool CreateEffectFromFileHelper(const char *effectname,
  const char *filename_vs, const char *filename_fs, const effect_defines_t *defines,
  std::vector<std::unique_ptr<RenderState>> states) {

  std::shared_ptr<GLShader> vs(new GLVertexShader), ps(new GLPixelShader);
  if (!vs->LoadFromFile(filename_vs, defines)) {
    return false;
  }
  if (!ps->LoadFromFile(filename_fs, defines)) {
    return false;
  }

  std::shared_ptr<GLEffect> effect(new GLEffect);
  if (!effect->AttachShaders(vs, ps)) {
    return false;
  }

  effect->SetRenderStates(std::move(states));

  auto &en = Engine::Instance();
  en.ResrcMgr().Add(effectname, bd_cast<Resource>(effect));
  en.CacheEffectFiles(effectname, filename_vs, filename_fs, defines);
  return true;
}

bool CreateEffectFromMemHelper(const char *effectname,
  const char *vs_buf, const char *ps_buf, const effect_defines_t *defines,
  std::vector<std::unique_ptr<RenderState>> states) {

  std::shared_ptr<GLShader> vs(new GLVertexShader), ps(new GLPixelShader);
  if (!vs->LoadFromMemory(vs_buf, defines)) {
    return false;
  }
  if (!ps->LoadFromMemory(ps_buf, defines)) {
    return false;
  }

  std::shared_ptr<GLEffect> effect(new GLEffect);
  if (!effect->AttachShaders(vs, ps)) {
    return false;
  }

  effect->SetRenderStates(std::move(states));

  Engine::Instance().ResrcMgr().Add(effectname, bd_cast<Resource>(effect));
  return true;
}

bool ReloadEffectFromFileHelper(const char *effectname,
  const char *filename_vs, const char *filename_fs,
  const effect_defines_t *defines) {

  auto &en = Engine::Instance();
  auto effect = bd_cast<GLEffect>(en.ResrcMgr().Find(effectname));
  if (!effect) {
    return false;
  }

  std::shared_ptr<GLShader> vs(new GLVertexShader), ps(new GLPixelShader);
  if (!vs->LoadFromFile(filename_vs, defines) || !ps->LoadFromFile(filename_fs, defines)) {
    return false;
  }
  if (!effect->AttachShaders(vs, ps)) {
    return false;
  }
  en.CacheEffectFiles(effectname, filename_vs, filename_fs, defines);
  return true;
}


} //namespace
