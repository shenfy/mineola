#include "prefix.h"
#include <mineola/TextLabel.h>
#include <numeric>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/VertexType.h>
#include <mineola/GraphicsBuffer.h>
#include <mineola/Font.h>
#include <mineola/Engine.h>

namespace mineola {

TextLabel::TextLabel(size_t max_length, const char *font_name)
  : max_length_(max_length),
  font_name_(font_name) {

  q_id_ = Renderable::kQueueOverlay;
  effect_name_ = "mineola:effect:text";

  BuildGeometry();
}

TextLabel::~TextLabel() {
}

void TextLabel::BuildGeometry() {
  using namespace mineola::vertex_type;

  auto max_num_verts = max_length_ * 4;

  positions_.resize(max_num_verts);
  uvs_.resize(max_num_verts);

  auto vs = std::make_shared<vertex_type::VertexStream>();
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 2});
  vs->type = VST_VERTEX;
  vs->size = max_num_verts;
  vs->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STREAM, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, glm::value_ptr(positions_[0]));

  auto ts = std::make_shared<vertex_type::VertexStream>();
  ts->layout.push_back({TEXCOORD0, type_mapping::FLOAT32, 2});
  ts->type = VST_VERTEX;
  ts->size = max_num_verts;
  ts->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STREAM, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  ts->buffer_ptr->Bind();
  ts->buffer_ptr->SetData(ts->Stride() * ts->size, glm::value_ptr(uvs_[0]));

  auto num_quads = max_length_;
  std::vector<uint32_t> indices(num_quads * 6);
  for (size_t idx = 0; idx < num_quads; idx++) {
    indices[idx * 6] = idx * 4;
    indices[idx * 6 + 1] = idx * 4 + 1;
    indices[idx * 6 + 2] = idx * 4 + 2;
    indices[idx * 6 + 3] = idx * 4;
    indices[idx * 6 + 4] = idx * 4 + 2;
    indices[idx * 6 + 5] = idx * 4 + 3;
  }

  auto is = std::make_shared<vertex_type::VertexStream>();
  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = indices.size();
  is->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER);
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, &indices[0]);

  auto va = std::make_shared<vertex_type::VertexArray>();
  va->PrimitiveType() = GL_TRIANGLES;
  va->AddVertexStream(vs);
  va->AddVertexStream(ts);
  va->SetIndexStream(is);

  pos_stream_ = vs;
  uv_stream_ = ts;

  auto font = Engine::Instance().FontMgr().Find(font_name_);
  if (font) {
    AddVertexArray(va, font->MaterialName().c_str());
  } else {
    AddVertexArray(va, "mineola:material:font");
  }
}

void TextLabel::SetText(const std::string &text) {
  if (text.size() > max_length_) {
    MLOG("Error: text length exceeded the label's capacity!\n");
    return;
  }

  auto &en = Engine::Instance();

  // get font
  auto font = font_.lock();
  if (!font) {
    font = en.FontMgr().Find(font_name_);
    if (!font) {
      return;
    }
    font_ = font;
  }

  // calc new layout, update CPU data
  font->Layout(text, positions_, uvs_);

  // fill empty data
  auto str_len = text.size();
  if (str_len < max_length_) {
    auto max_num_verts = max_length_ * 4;
    for (size_t idx = str_len * 4; idx < max_num_verts; ++idx) {
      positions_[idx] = glm::vec2(0.f, 0.f);
      uvs_[idx] = glm::vec2(0.f, 0.f);
    }
  }

  // update GPU data
  auto vs = pos_stream_.lock();
  if (vs) {
    vs->buffer_ptr->Bind();
    vs->buffer_ptr->UpdateData(0, vs->Stride() * vs->size, glm::value_ptr(positions_[0]));
  }

  auto ts = uv_stream_.lock();
  if (ts) {
    ts->buffer_ptr->Bind();
    ts->buffer_ptr->UpdateData(0, ts->Stride() * ts->size, glm::value_ptr(uvs_[0]));
  }
}

}