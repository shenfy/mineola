#include "prefix.h"
#include <cstdio>
#include <mineola/GraphicsBuffer.h>
#include <mineola/TypeMapping.h>
#include <mineola/glutility.h>

namespace mineola {

  GraphicsBuffer::GraphicsBuffer(uint32_t frequency,
    uint32_t direction,
    uint32_t access,
    uint32_t target)
    : buffer_handle_(0),
      frequency_(frequency),
      direction_(direction),
      access_(access),
      targets_({target}),
      index_(0) {
    glGenBuffers(1, &buffer_handle_);
    CHKGLERR
  }

  GraphicsBuffer::GraphicsBuffer(uint32_t frequency,
    uint32_t direction,
    uint32_t access,
    std::vector<uint32_t> targets)
    : buffer_handle_(0),
      frequency_(frequency),
      direction_(direction),
      access_(access),
      targets_(std::move(targets)),
      index_(0) {
    glGenBuffers(1, &buffer_handle_);
    CHKGLERR
  }

  GraphicsBuffer::GraphicsBuffer(uint32_t frequency,
    uint32_t direction,
    uint32_t access,
    uint32_t target,
    uint32_t index)
    : buffer_handle_(0),
      frequency_(frequency),
      direction_(direction),
      access_(access),
      targets_({target}),
      index_(index) {
    glGenBuffers(1, &buffer_handle_);
    CHKGLERR
  }

  GraphicsBuffer::~GraphicsBuffer() {
    if (buffer_handle_) {
      glDeleteBuffers(1, &buffer_handle_);
    }
  }

  uint32_t GraphicsBuffer::Handle() {
    return buffer_handle_;
  }

  void GraphicsBuffer::Bind() {
    for (auto target : targets_) {
      glBindBuffer(target, buffer_handle_);
    }
  }

  void GraphicsBuffer::BindBase() {
    glBindBufferBase(targets_[0], index_, buffer_handle_);
  }

  void GraphicsBuffer::Unbind() {
    for (auto target : targets_) {
      glBindBuffer(target, 0);
    }
  }

  void GraphicsBuffer::SetBindTargets(std::vector<uint32_t> targets) {
    targets_ = std::move(targets);
  }

  bool GraphicsBuffer::SetSize(uint32_t size) {
    glBindBuffer(targets_[0], buffer_handle_);
    glBufferData(targets_[0], size, 0, type_mapping::Usage2GL(frequency_, direction_));
    glBindBuffer(targets_[0], 0);
    return true;
  }

  bool GraphicsBuffer::SetData(uint32_t size, const void *data) {
    glBindBuffer(targets_[0], buffer_handle_);
    glBufferData(targets_[0], size, data, type_mapping::Usage2GL(frequency_, direction_));
    glBindBuffer(targets_[0], 0);
    return true;
  }

  bool GraphicsBuffer::UpdateData(uint32_t offset, uint32_t size, const void *data) {
    glBindBuffer(targets_[0], buffer_handle_);
    glBufferSubData(targets_[0], offset, size, data);
    glBindBuffer(targets_[0], 0);
    return true;
  }
}
