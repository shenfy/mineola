#include "prefix.h"
#include "../include/UniformBlock.h"
#include <string>
#include "../include/glutility.h"
#include "../include/GraphicsBuffer.h"

namespace mineola {

const char *UniformBlock::GetSemanticsString(int semantics) {
  switch (semantics) {
  case BUILTIN_UNIFORMS:
    return "mineola_builtin_uniforms";
  }
  return nullptr;
}

uint32_t UniformBlock::GetSemanticsBindLocation(int semantics) {
  switch (semantics) {
  case BUILTIN_UNIFORMS:
    return 0;
  }
  return -1;
}

UniformBlock::UniformBlock(uint32_t block_index) {
  buffer_ = std::make_unique<GraphicsBuffer>(GraphicsBuffer::DYNAMIC,
    GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_UNIFORM_BUFFER, block_index);
}

UniformBlock::~UniformBlock() {
}

bool UniformBlock::SetSize(uint32_t size) {
  return buffer_->SetSize(size);
}

void UniformBlock::Activate() {
  buffer_->BindBase();
}

void UniformBlock::SetVariable(const char *variable_name, uint32_t offset, uint32_t size) {
  variable_map_[variable_name] = {offset, size};
}

bool UniformBlock::UpdateVariable(const char *variable_name, const void *data) {
  auto iter = variable_map_.find(variable_name);
  if (iter != variable_map_.end()) {
    return buffer_->UpdateData(iter->second.first, iter->second.second, data);
  }
  return false;
}

}
