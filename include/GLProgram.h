#ifndef MINEOLA_GLPROGRAM_H
#define MINEOLA_GLPROGRAM_H

#include <string>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include "BasisObj.h"

namespace mineola {

class GLProgram : boost::noncopyable, public Resource {
public:
  GLProgram();
  virtual ~GLProgram();

  bool Bind();

  void UploadVariable(const char *var_name, const float *val);
  void UploadVariable(const char *var_name, const int32_t *val);
  void UploadVariable(const char *var_name, const uint32_t *val);

  // returns -1 if not found
  int32_t GetSSBBinding(const std::string &name);
  int32_t GetUniformBlockBinding(const std::string &name);

protected:
  bool InfoLog() const;

  bool GetVariableInfo(const char *name,
    uint32_t &type, uint32_t &loc, uint32_t &length);

  bool GenerateVarMap();
  bool GenerateUniformBlockMap();
  void BindBuiltinUniformBlocks();

  uint32_t handle_;

  //map from var/attrib name to location and type
  std::unordered_map<std::string,
    std::tuple<uint32_t/*loc*/, uint32_t/*type*/, uint32_t/*size*/>> variable_map_;

  // map from shader storage buffer name to binding index
  std::unordered_map<std::string, int32_t> ssb_map_;

  // map from uniform block name to pair(block idx, binding idx)
  std::unordered_map<std::string, std::pair<int32_t, int32_t>> ub_map_;

  enum {VAR_NAME_MAX_LENGTH = 30};
};

} //namespace

#endif
