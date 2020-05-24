#ifndef MINEOLA_GLSHADER_H
#define MINEOLA_GLSHADER_H

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <unordered_map>
#include "Noncopyable.h"

namespace mineola {

class GLShader : Noncopyable {
public:
  typedef std::vector<std::pair<std::string, std::string>> effect_defines_t;

public:
  GLShader();
  virtual ~GLShader();
  uint32_t Handle() const {return handle_;}

  bool LoadFromMemory(const char *pbuf, const effect_defines_t *defines);
  bool LoadFromFile(const char *fn, const effect_defines_t *defines);

protected:
  bool LoadFromRawMemory(const char *str);
  bool InfoLog() const;

  uint32_t handle_;
};

class GLVertexShader : public GLShader {
public:
  GLVertexShader();
  virtual ~GLVertexShader();
};

class GLPixelShader : public GLShader {
public:
  GLPixelShader();
  virtual ~GLPixelShader();
};

} //namespace

#endif
