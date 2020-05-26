#include "prefix.h"
#include "../include/GLShader.h"
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <fstream>
#include "../include/ShaderParser.h"
#include "../include/Engine.h"
#include "../include/glutility.h"

namespace {
  std::string AddLineNumber(const std::string &shader_str) {
    std::string result;
    int line = 1;
    result += std::to_string(line) + " ";
    for (const char c : shader_str) {
      result.push_back(c);
      if (c == '\n') {
        result += std::to_string(++line) + " ";
      }
    }
    return result;
  }
}

namespace mineola {

GLShader::GLShader()
  : handle_(0) {}

GLShader::~GLShader() {
  if (handle_)
    glDeleteShader(handle_);
}

bool GLShader::LoadFromMemory(const char *pbuf, const effect_defines_t *defines) {
  std::istringstream stream(pbuf);
  std::string shader_str;
  if (!shader_parser::ParseShader(stream, defines, shader_str))
    return false;
  return LoadFromRawMemory(shader_str.c_str());
}

bool GLShader::LoadFromFile(const char *fn, const effect_defines_t *defines) {
  std::string found_fn;
  if (!Engine::Instance().ResrcMgr().LocateFile(fn, found_fn)) {
    MLOG("[%s] does not exists!\n", fn);
    return false;
  }

  std::ifstream infile(found_fn);
  if (!infile.good()) {
    MLOG("Failed to open [%s]!\n", found_fn.c_str());
    return false;
  }

  std::string shader_str;
  if (!shader_parser::ParseShader(infile, defines, shader_str))
    return false;
  return LoadFromRawMemory(shader_str.c_str());
}

bool GLShader::LoadFromRawMemory(const char *str) {
  if (handle_ == 0) {
    MLOG("Unable to load code to an invalid GLShader!\n");
    return false;
  }

  glShaderSource(handle_, 1, (const char **)&str, NULL);
  glCompileShader(handle_);
  bool result = InfoLog();
  if (!result) {
    MLOG("%s", AddLineNumber(str).c_str());
  }
  return result;
}

bool GLShader::InfoLog() const {
  int infologLength = 0;
  int charsWritten  = 0;
  static char infoLog[512];

  glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 0) {
    glGetShaderInfoLog(handle_, std::min(infologLength, 512), &charsWritten, infoLog);
    if (charsWritten == 0)
      return true;
    MLOG("%s\n", infoLog);
    return false;
  }
  return true;
}

GLVertexShader::GLVertexShader() {
  handle_ = glCreateShader(GL_VERTEX_SHADER);
}

GLVertexShader::~GLVertexShader() {}

GLPixelShader::GLPixelShader() {
  handle_ = glCreateShader(GL_FRAGMENT_SHADER);
}

GLPixelShader::~GLPixelShader() {}

} //namespace
