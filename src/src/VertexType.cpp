#include <mineola/VertexType.h>
#include <mineola/GLEffect.h>
#include <mineola/GraphicsBuffer.h>
#include <mineola/glutility.h>

namespace mineola { namespace vertex_type {

uint32_t LayoutElement::SizeOf() const {
  return type_mapping::SizeOf(format) * length;
}

uint32_t VertexStream::Stride() {
  if (force_stride > 0) {
    return force_stride;
  }
  uint32_t stride = 0;
  for (auto iter = layout.begin(); iter != layout.end(); ++iter) {
    stride += iter->SizeOf();
  }
  return stride;
}

VertexArray::VertexArray() :
  vao_updated_(false),
  primitive_type_(GL_TRIANGLES),
  is_indexed_(false) {
}

VertexArray::~VertexArray() {
  vertex_stream_ptrs_.clear();
}

int &VertexArray::PrimitiveType() {
  return primitive_type_;
}

void VertexArray::AddVertexStream(std::shared_ptr<VertexStream> vertex_stream_ptr) {
  vertex_stream_ptrs_.push_back(std::move(vertex_stream_ptr));
  vao_updated_ = false;
}

void VertexArray::SetIndexStream(std::shared_ptr<VertexStream> index_stream_ptr) {
  index_stream_ptr_ = std::move(index_stream_ptr);
  vao_updated_ = false;
  is_indexed_ = true;
}

void VertexArray::SetIndexed(bool indexed) {
  is_indexed_ = indexed;
}

void VertexArray::MarkVertexUpdated() {
  vao_updated_ = false;
}

bool VertexArray::UpdateVAO() {
  if (!vao_ptr_) {
    vao_ptr_.reset(new VertexArrayObject);
  }

  if (!vao_ptr_) {
    return false;
  }

  vao_ptr_->Bind();
  //bind vertex buffers
  std::shared_ptr<GraphicsBuffer> last_buffer;
  for (auto &vertex_stream_ptr : vertex_stream_ptrs_) {
    // avoid repeated binding the same buffer
    if (vertex_stream_ptr->buffer_ptr != last_buffer) {
      vertex_stream_ptr->buffer_ptr->Bind();
      last_buffer = vertex_stream_ptr->buffer_ptr;
    }

    //bind attributes
    uint32_t offset = vertex_stream_ptr->offset;
    uint32_t stride = vertex_stream_ptr->Stride();
    for (const auto &layout : vertex_stream_ptr->layout) {
      int bind_loc = GetSemanticsBindLocation(layout.semantics);
      if (bind_loc >= 0) {
        glEnableVertexAttribArray(bind_loc);
        glVertexAttribPointer(bind_loc, layout.length, type_mapping::Map2GLType(layout.format),
          GL_FALSE, stride, reinterpret_cast<GLvoid*>((uintptr_t)offset));
      }
      offset += layout.SizeOf();
    }
  }

  //bind index buffer
  if (index_stream_ptr_ && index_stream_ptr_->buffer_ptr != last_buffer) {
    index_stream_ptr_->buffer_ptr->Bind();
  }

  vao_ptr_->Unbind();

  vao_updated_ = true;
  return true;
}

bool VertexArray::Draw() {
  if (!vao_updated_ && !UpdateVAO()) {
    return false;
  }

  if (vao_ptr_) {
    vao_ptr_->Bind();

    CHKGLERR_RET

    if (is_indexed_ && index_stream_ptr_) {
      // render
      glDrawElements(primitive_type_, index_stream_ptr_->size,
        type_mapping::Map2GLType(index_stream_ptr_->layout[0].format),
        reinterpret_cast<GLvoid*>((uintptr_t)index_stream_ptr_->offset));
    } else {
      glDrawArrays(primitive_type_, 0, vertex_stream_ptrs_[0]->size);
    }

    CHKGLERR_RET

    vao_ptr_->Unbind();
  }
  return true;
}

VertexArrayObject::VertexArrayObject() :
  vao_handle_(0) {
  CHKGLERR
  glGenVertexArrays(1, &vao_handle_);
  CHKGLERR
}

VertexArrayObject::~VertexArrayObject() {
  if (vao_handle_ != 0)
      glDeleteVertexArrays(1, &vao_handle_);
}

void VertexArrayObject::Bind() {
  glBindVertexArray(vao_handle_);
}

void VertexArrayObject::Unbind() {
  glBindVertexArray(0);
}

}} //namespace
