#include "prefix.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unordered_set>
#include <numeric>
#include <boost/assert.hpp>
#include "../include/glutility.h"
#include "../include/GLShader.h"
#include "../include/GLProgram.h"
#include "../include/TypeMapping.h"
#include "../include/UniformBlock.h"
#include "../include/UniformHelper.h"

namespace mineola {

GLProgram::GLProgram()
  : handle_(glCreateProgram()) {}

GLProgram::~GLProgram() {
  if (handle_)
    glDeleteProgram(handle_);
}

bool GLProgram::InfoLog() const {
  int log_length = 0;
  int num_chars  = 0;
  static char log_str[512];

  glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &log_length);

  if (log_length > 0) {
    glGetProgramInfoLog(handle_, std::min(log_length, 512), &num_chars, log_str);
    if (strlen(log_str) > 0) {
      MLOG("%s\n", log_str);
      return false;
    }
  }
  return true;
}

// this forces the built-in uniform block to bind to binding point 0
void GLProgram::BindBuiltinUniformBlocks() {
  for (int semantics = UniformBlock::BUILTIN_UNIFORMS;
    semantics < UniformBlock::RESERVED_SEMANTICS_NUM; ++semantics) {

    const char *block_name = UniformBlock::GetSemanticsString(semantics);
    int bind_loc = UniformBlock::GetSemanticsBindLocation(semantics);
    auto iter = ub_map_.find(block_name);
    if (iter != ub_map_.end()) {
      glUniformBlockBinding(handle_, iter->second.first, bind_loc);
    }
  }
}
  
bool GLProgram::GenerateUniformBlockMap() {
  BOOST_ASSERT(handle_);

  ub_map_.clear();

  int32_t num_ubs = 0;
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORM_BLOCKS, &num_ubs);
  if (num_ubs <= 0)
    return true;
  
  static char name[VAR_NAME_MAX_LENGTH] = {0};
  int32_t name_length = 0;
  int32_t binding = -1;
  
  for (int32_t i = 0; i < num_ubs; ++i) {
    glGetActiveUniformBlockName(handle_, i, VAR_NAME_MAX_LENGTH, &name_length, name);
    glGetActiveUniformBlockiv(handle_, i, GL_UNIFORM_BLOCK_BINDING, &binding);
    ub_map_[name] = std::make_pair(i, binding);
  }

  return true;
}

bool GLProgram::GenerateVarMap() {
  BOOST_ASSERT(handle_);
  
  variable_map_.clear();
  int32_t num_uniforms = 0;
  glGetProgramiv(handle_, GL_ACTIVE_UNIFORMS, &num_uniforms);
  if (num_uniforms <= 0)
    return true;
  
  static char uniform_name[VAR_NAME_MAX_LENGTH] = {0};
  int32_t uniform_name_length = 0, uniform_size = 0, uniform_loc = 0;
  GLenum uniform_type;
  
  std::vector<GLuint> uniform_indices(num_uniforms);
  std::iota(uniform_indices.begin(), uniform_indices.end(), 0);
  std::vector<GLint> block_indices(num_uniforms);
  
  glGetActiveUniformsiv(handle_, num_uniforms, &uniform_indices[0], GL_UNIFORM_BLOCK_INDEX, &block_indices[0]);
  
  for (int32_t i = 0; i < num_uniforms; ++i) {
    if (block_indices[i] != -1)
      continue;
    
    glGetActiveUniform(handle_, i, VAR_NAME_MAX_LENGTH,
      &uniform_name_length, &uniform_size, &uniform_type, uniform_name);
    uniform_loc = glGetUniformLocation(handle_, uniform_name);
    if (uniform_loc != -1) {
      variable_map_[uniform_name] = std::make_tuple(uniform_loc, (uint32_t)uniform_type, uniform_size);
    }
  }
  
//  for (auto iter = variable_map_.begin(); iter != variable_map_.end(); ++iter)
//    printf("%s: %d, %d, %d\n", iter->first.c_str(), std::get<0>(iter->second), std::get<1>(iter->second), std::get<2>(iter->second));
  
  return true;
}
  
int32_t GLProgram::GetUniformBlockBinding(const std::string &name) {
  auto iter = ub_map_.find(name);
  if (iter != ub_map_.end()) {
    return iter->second.second;
  }
  return -1;
}


bool GLProgram::GetVariableInfo(
  const char *name,
  uint32_t &type, uint32_t &loc, uint32_t &length) {

  auto iter = variable_map_.find(name);
  if (iter != variable_map_.end()) {
    type = std::get<1>(iter->second);
    loc = std::get<0>(iter->second);
    length = std::get<2>(iter->second);
    return true;
  }
  return false;
}

void GLProgram::UploadVariable(const char *var_name, const float *val) {
  uint32_t type = 0, loc = 0, length = 0;
  if (GetVariableInfo(var_name, type, loc, length)) {
    gl_uniform::SetUniform(type, loc, length, val);        
  }
}

void GLProgram::UploadVariable(const char *var_name, const int32_t *val) {
  uint32_t type = 0, loc = 0, length = 0;
  if (GetVariableInfo(var_name, type, loc, length)) {
    gl_uniform::SetUniform(type, loc, length, val);        
  }
}

void GLProgram::UploadVariable(const char *var_name, const uint32_t *val) {
  uint32_t type = 0, loc = 0, length = 0;
  if (GetVariableInfo(var_name, type, loc, length)) {
    gl_uniform::SetUniform(type, loc, length, val);        
  }
}

bool GLProgram::Bind() {
  BOOST_ASSERT(handle_);
  CHKGLERR_RET;
  glUseProgram(handle_);
  CHKGLERR_RET;
  return true;
}

} //namespace
