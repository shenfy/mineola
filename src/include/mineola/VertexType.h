#ifndef MINEOLA_VERTEXTYPE_H
#define MINEOLA_VERTEXTYPE_H
#include <vector>
#include <memory>
#include "TypeMapping.h"
#include "Noncopyable.h"

namespace mineola {

class GraphicsBuffer;
class GLEffect;
class VertexArrayObject;

namespace vertex_type {

enum Semantics {
  UNKNOWN = 0, POSITION, NORMAL, TEXCOORD0, TEXCOORD1, TEXCOORD2,
  DIFFUSE_COLOR, SPECULAR_COLOR, BLEND_WEIGHT, BLEND_INDEX, TANGENT, BINORMAL, INDEX
};

enum { RESERVED_SEMANTICS_NUM = 12 };

enum StreamType { VST_VERTEX, VST_INDEX };

inline const char *GetSemanticsString(uint32_t semantics) {
  switch (semantics) {
  case POSITION: return "Pos";
  case NORMAL: return "Normal";
  case TEXCOORD0: return "TexCoord0";
  case TEXCOORD1: return "TexCoord1";
  case TEXCOORD2: return "TexCoord2";
  case DIFFUSE_COLOR: return "Diffuse";
  case SPECULAR_COLOR: return "Specular";
  case BLEND_INDEX: return "BlendIdx";
  case BLEND_WEIGHT: return "BlendWeight";
  case TANGENT: return "Tangent";
  case BINORMAL: return "Binormal";
  case INDEX: return "Index";
  default: return nullptr;
  }
}

inline const int GetSemanticsBindLocation(uint32_t semantics) {
  switch (semantics) {
  case POSITION: return 0;
  case NORMAL: return 1;
  case TEXCOORD0: return 2;
  case TEXCOORD1: return 3;
  case TEXCOORD2: return 4;
  case DIFFUSE_COLOR: return 5;
  case SPECULAR_COLOR: return 6;
  case BLEND_INDEX: return 7;
  case BLEND_WEIGHT: return 8;
  case TANGENT: return 9;
  case BINORMAL: return 10;
  case INDEX: return 11;
  default: return -1;
  }
}

struct LayoutElement {
  uint32_t SizeOf() const;

  uint32_t semantics{UNKNOWN};
  uint32_t format{UNKNOWN};
  uint32_t length{0};
};

struct VertexStream {
  uint32_t Stride();

  std::vector<LayoutElement> layout;
  uint32_t type{VST_VERTEX};
  uint32_t size{0};
  uint32_t offset{0};
  uint32_t force_stride{0};
  std::shared_ptr<GraphicsBuffer> buffer_ptr;
};

class VertexArrayObject : Noncopyable {
public:
  VertexArrayObject();
  ~VertexArrayObject();

  void Bind();
  void Unbind();

protected:
  uint32_t vao_handle_;
};

class VertexArray {
public:
  VertexArray();
  ~VertexArray();

  void AddVertexStream(std::shared_ptr<VertexStream> vertex_stream);
  void SetIndexStream(std::shared_ptr<VertexStream> index_stream);

  std::vector<std::shared_ptr<VertexStream>> &VertexStreams();
  void MarkVertexUpdated();  // MUST call after updating vertex data
  bool Draw();

  int &PrimitiveType();

  void SetIndexed(bool indexed);

protected:
  bool UpdateVAO();

  std::vector<std::shared_ptr<VertexStream>> vertex_stream_ptrs_;
  std::shared_ptr<VertexStream> index_stream_ptr_;
  std::shared_ptr<VertexArrayObject> vao_ptr_;
  bool vao_updated_;
  int primitive_type_;
  bool is_indexed_;
};

}} //namespaces

#endif
