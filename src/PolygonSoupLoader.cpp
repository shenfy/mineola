#include "prefix.h"
#include "../include/PolygonSoupLoader.h"
#include <numeric>
#include "../include/GraphicsBuffer.h"
#include "../include/Renderable.h"
#include "../include/Material.h"
#include "../include/PolygonSoup.h"
#include "../include/TextureHelper.h"
#include "../include/Engine.h"

namespace mineola { namespace primitive_helper {

bool BuildFromPolygonSoup(const PolygonSoup &soup,
  const char *name,
  const char *effect_name,
  Renderable &renderable) {
  // verts
  using namespace mineola::vertex_type;

  std::vector<float> verts_data;
  size_t num_attributes = 3 + (soup.has_vertex_normal ? 3 : 0)
    + (soup.has_vertex_texcoord ? 2 : 0);
  verts_data.reserve(num_attributes * soup.vertices.size());
  for (const auto &vert: soup.vertices) {
    verts_data.push_back(vert.pos[0]);
    verts_data.push_back(vert.pos[1]);
    verts_data.push_back(vert.pos[2]);
    if (soup.has_vertex_normal) {
      verts_data.push_back(vert.normal[0]);
      verts_data.push_back(vert.normal[1]);
      verts_data.push_back(vert.normal[2]);
    }
    if (soup.has_vertex_texcoord) {
      verts_data.push_back(vert.tex[0]);
      verts_data.push_back(vert.tex[1]);
    }
    if (soup.has_vertex_color) {
      // encode rgba to float
      verts_data.push_back(*(float*)&vert.color.val);
    }
  }

  std::shared_ptr<VertexArray> va(new VertexArray());

  std::shared_ptr<VertexStream> vs(new VertexStream());
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  if (soup.has_vertex_normal) {
    vs->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  }
  if (soup.has_vertex_texcoord) {
    vs->layout.push_back({TEXCOORD0, type_mapping::FLOAT32, 2});
  }
  if (soup.has_vertex_color) {
    vs->layout.push_back({DIFFUSE_COLOR, type_mapping::UBYTE, 4});
  }
  vs->type = VST_VERTEX;
  vs->size = (uint32_t)soup.vertices.size();
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, &verts_data[0]);

  verts_data.clear();


  std::shared_ptr<VertexStream> is(new VertexStream());
  // prioritize edges
  if (soup.edges.size() != 0) {
    std::vector<uint32_t> edges_data;
    edges_data.reserve(soup.edges.size() * 2);
    for (const auto &edge: soup.edges) {
      edges_data.push_back((uint32_t)edge.first);
      edges_data.push_back((uint32_t)edge.second);
    }

    is->layout.push_back({INDEX, type_mapping::UINT32, 1});
    is->type = VST_INDEX;
    is->size = (uint32_t)soup.edges.size() * 2;
    is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
      GraphicsBuffer::SEND,
      GraphicsBuffer::READ_ONLY,
      GL_ELEMENT_ARRAY_BUFFER));
    is->buffer_ptr->Bind();
    is->buffer_ptr->SetData(is->Stride() * is->size, &edges_data[0]);
    edges_data.clear();
    va->PrimitiveType() = GL_LINES;
  } else if (soup.faces.size() != 0) {
    // draw triangles
    std::vector<uint32_t> faces_data;
    faces_data.reserve(soup.faces.size() * 3);
    size_t index_count = 0;
    for (const auto &face: soup.faces) {
      if (face.indices.size() != 3)  // skip non-triangle faces
        continue;

      for (size_t index : face.indices) {
        faces_data.push_back((uint32_t)index);
      }
      index_count += face.indices.size();
    }

    is->layout.push_back({INDEX, type_mapping::UINT32, 1});
    is->type = VST_INDEX;
    is->size = (uint32_t)index_count;
    is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
      GraphicsBuffer::SEND,
      GraphicsBuffer::READ_ONLY,
      GL_ELEMENT_ARRAY_BUFFER));
    is->buffer_ptr->Bind();
    is->buffer_ptr->SetData(is->Stride() * is->size, &faces_data[0]);
    faces_data.clear();
    va->PrimitiveType() = GL_TRIANGLES;
  } else {
    // draw points
    std::vector<uint32_t> indices(soup.vertices.size());
    std::iota(indices.begin(), indices.end(), 0);

    is->layout.push_back({INDEX, type_mapping::UINT32, 1});
    is->type = VST_INDEX;
    is->size = (uint32_t)indices.size();
    is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
      GraphicsBuffer::SEND,
      GraphicsBuffer::READ_ONLY,
      GL_ELEMENT_ARRAY_BUFFER));
    is->buffer_ptr->Bind();
    is->buffer_ptr->SetData(is->Stride() * is->size, &indices[0]);
    indices.clear();
    va->PrimitiveType() = GL_POINTS;
  }

  va->AddVertexStream(vs);
  va->SetIndexStream(is);

  // create texture
  if (!soup.texture_filename.empty())
   texture_helper::CreateTexture(soup.texture_filename.c_str(),
     soup.texture_filename.c_str(), false, true);

  // create material
  auto material = std::make_shared<Material>();
  material->alpha = 1.0f;
  material->specularity = 30.0f;
  material->ambient = glm::vec3(0.f, 0.f, 0.f);
  material->diffuse = glm::vec3(1.f, 1.f, 1.f);
  material->specular = glm::vec3(0.2f, 0.2f, 0.2f);
  material->emit = glm::vec3(0.f, 0.f, 0.f);
  if (!soup.texture_filename.empty()) {
    material->texture_slots["diffuse_sampler"] = {soup.texture_filename};
  }
  std::string material_name = "mat:";
  material_name.append(name);
  Engine::Instance().ResrcMgr().Add(material_name, material);

  // fill renderable
  renderable.AddVertexArray(va, material_name.c_str());
  renderable.SetEffect(effect_name);
  return true;
}


}}  // namespaces
