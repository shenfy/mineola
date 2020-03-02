#ifndef MINEOLA_UNIFORMBLOCK_H
#define MINEOLA_UNIFORMBLOCK_H

#include <memory>
#include <unordered_map>
#include "BasisObj.h"

namespace mineola {

class GraphicsBuffer;

class UniformBlock : public Resource {
public:
  enum Semantics { BUILTIN_UNIFORMS = 0, RESERVED_SEMANTICS_NUM = 1 };
  static const char *GetSemanticsString(int semantics);
  static uint32_t GetSemanticsBindLocation(int semantics);

public:
  explicit UniformBlock(uint32_t block_index);
  virtual ~UniformBlock();

  bool SetSize(uint32_t size);
  void Activate();
  void SetVariable(const char *variable_name, uint32_t offset, uint32_t size);
  bool UpdateVariable(const char *variable_name, const void *data);

protected:
  std::unique_ptr<GraphicsBuffer> buffer_;
  std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> variable_map_;
};

}

#endif
