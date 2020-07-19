#ifndef MINEOLA_TEXT_H
#define MINEOLA_TEXT_H

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <mineola/Renderable.h>

namespace mineola {

namespace vertex_type {
class VertexStream;
}

class Font;

class TextLabel : public Renderable {
public:
  TextLabel(size_t max_length, const char *font_name);
  virtual ~TextLabel() override;

  void SetText(const std::string &text);

protected:
  void BuildGeometry();

  const size_t max_length_;
  std::string font_name_;
  std::weak_ptr<Font> font_;
  std::vector<glm::vec2> positions_;
  std::weak_ptr<vertex_type::VertexStream> pos_stream_;
  std::vector<glm::vec2> uvs_;
  std::weak_ptr<vertex_type::VertexStream> uv_stream_;
};

} //namespaces

#endif