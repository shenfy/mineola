#include "prefix.h"
#include <mineola/PrimitiveHelper.h>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/GraphicsBuffer.h>

namespace mineola { namespace primitive_helper {

void BuildRect(float size, vertex_type::VertexArray &vertex_array) {
  std::vector<float> vertices;
  std::vector<int> indices;

  static const float verts[] = {
    -1.f, 0.f, 1.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 0.f, 0.f,
    1.f, 0.f, 1.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 1.f, 0.f,
    1.f, 0.f, -1.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 1.f, 1.f,
    -1.f, 0.f, -1.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 0.f, 1.f};

  static const int inds[] = {0, 1, 2, 0, 2, 3};
  for (int i = 0; i < 32; ++i) {
    if (i % 8 < 3)
      vertices.push_back(verts[i] * size / 2.f);
    else
      vertices.push_back(verts[i]);
  }
  for (int i = 0; i < 6; ++i)
    indices.push_back(inds[i]);

  using namespace mineola::vertex_type;
  std::shared_ptr<VertexStream> vs(new VertexStream);
  std::shared_ptr<VertexStream> is(new VertexStream);
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  vs->layout.push_back({TEXCOORD0, type_mapping::FLOAT32, 2});
  vs->type = VST_VERTEX;
  vs->size = 4;
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, &vertices[0]);

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = 6;
  is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER));
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, &indices[0]);

  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}

void BuildRectXY(float size, vertex_type::VertexArray &vertex_array) {
  std::vector<float> vertices;
  std::vector<int> indices;

  static const float verts[] = {
    -1.f, 1.f, 0.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 0.f, 0.f,
    1.f, 1.f, 0.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 1.f, 0.f,
    1.f, -1.f, 0.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 1.f, 1.f,
    -1.f, -1.f, 0.f, /*normal*/ 0.f, 1.f, 0.f, /*texcoord*/ 0.f, 1.f};

  static const int inds[] = {0, 1, 2, 0, 2, 3};
  for (int i = 0; i < 32; ++i) {
    if (i % 8 < 3)
      vertices.push_back(verts[i] * size / 2.f);
    else
      vertices.push_back(verts[i]);
  }
  for (int i = 0; i < 6; ++i)
    indices.push_back(inds[i]);

  using namespace mineola::vertex_type;
  std::shared_ptr<VertexStream> vs(new VertexStream);
  std::shared_ptr<VertexStream> is(new VertexStream);
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  vs->layout.push_back({TEXCOORD0, type_mapping::FLOAT32, 2});
  vs->type = VST_VERTEX;
  vs->size = 4;
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, &vertices[0]);

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = 6;
  is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER));
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, &indices[0]);

  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}

void BuildSphere(int sub_div_level, vertex_type::VertexArray &vertex_array) {
  std::vector<glm::vec3> vertices;
  std::vector<glm::ivec3> triangles;

  // icosahedron
  enum Index {ZA, ZB, ZC, ZD, YA, YB, YC, YD, XA, XB, XC, XD };

  float t = ( 1.0f + sqrt( 5.0f) ) / 2.0f;
  float o = 1.0f / sqrt( 1.0f + t*t );
  float z = 0.0f;
  t *= 1.0f * o;
  o *= 1.0f;

  vertices.push_back( glm::vec3( t, o, z ) );
  vertices.push_back( glm::vec3(-t, o, z ) );
  vertices.push_back( glm::vec3(-t,-o, z ) );
  vertices.push_back( glm::vec3( t,-o, z ) );
  vertices.push_back( glm::vec3( o, z, t ) );
  vertices.push_back( glm::vec3( o, z,-t ) );
  vertices.push_back( glm::vec3(-o, z,-t ) );
  vertices.push_back( glm::vec3(-o, z, t ) );
  vertices.push_back( glm::vec3( z, t, o ) );
  vertices.push_back( glm::vec3( z,-t, o ) );
  vertices.push_back( glm::vec3( z,-t,-o ) );
  vertices.push_back( glm::vec3( z, t,-o ) );

  triangles.push_back( glm::ivec3( YA, XA, YD ) );
  triangles.push_back( glm::ivec3( YA, YD, XB ) );
  triangles.push_back( glm::ivec3( YB, YC, XD ) );
  triangles.push_back( glm::ivec3( YB, XC, YC ) );
  triangles.push_back( glm::ivec3( ZA, YA, ZD ) );
  triangles.push_back( glm::ivec3( ZA, ZD, YB ) );
  triangles.push_back( glm::ivec3( ZC, YD, ZB ) );
  triangles.push_back( glm::ivec3( ZC, ZB, YC ) );
  triangles.push_back( glm::ivec3( XA, ZA, XD ) );
  triangles.push_back( glm::ivec3( XA, XD, ZB ) );
  triangles.push_back( glm::ivec3( XB, XC, ZD ) );
  triangles.push_back( glm::ivec3( XB, ZC, XC ) );
  triangles.push_back( glm::ivec3( XA, YA, ZA ) );
  triangles.push_back( glm::ivec3( XD, ZA, YB ) );
  triangles.push_back( glm::ivec3( YA, XB, ZD ) );
  triangles.push_back( glm::ivec3( YB, ZD, XC ) );
  triangles.push_back( glm::ivec3( YD, XA, ZB ) );
  triangles.push_back( glm::ivec3( YC, ZB, XD ) );
  triangles.push_back( glm::ivec3( YD, ZC, XB ) );
  triangles.push_back( glm::ivec3( YC, XC, ZC ) );

  typedef std::pair< int, int > Edge;

  for (int level = 0; level < sub_div_level; ++level) {
    int numTris = (int)triangles.size();
    std::map< Edge, int > edgeToVertexIndexMap;
    std::pair< std::map< Edge, int>::iterator, bool > ret;
    for( int triIndex = 0; triIndex < numTris; triIndex++ ) {
      const glm::ivec3 & curTri = triangles[ triIndex ];

      int vertIndex0 = curTri[0];
      //const Vec3& vert0 = vertices[ vertIndex0 ];
      int vertIndex1 = curTri[1];
      //const Vec3& vert1 = vertices[ vertIndex1 ];
      int vertIndex2 = curTri[2];
      //const Vec3& vert2 = vertices[ vertIndex2 ];

      // handle edge 0-1
      int vertIndex3;
      if ( vertIndex0 < vertIndex1 ) {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex0, vertIndex1), (int)vertices.size() ) );
      } else {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex1, vertIndex0), (int)vertices.size() ) );
      }

      if (ret.second) {
        glm::vec3 vert3 = vertices[ vertIndex0 ] + vertices[ vertIndex1 ];
        vert3 = glm::normalize(vert3);
        vertIndex3 = (int)vertices.size();
        vertices.push_back( vert3 );
      } else {
        vertIndex3 = ret.first->second;
      }

      // handle edge 1-2
      int vertIndex4;
      if ( vertIndex1 < vertIndex2 ) {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex1, vertIndex2), (int)vertices.size() ) );
      } else {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex2, vertIndex1), (int)vertices.size() ) );
      }

      if (ret.second) {
        glm::vec3 vert4 = vertices[ vertIndex1 ] + vertices[ vertIndex2 ];
        vert4 = glm::normalize(vert4);
        vertIndex4 = (int)vertices.size();
        vertices.push_back( vert4 );
      } else {
        vertIndex4 = ret.first->second;
      }

      // handle edge 1-3
      int vertIndex5;
      if ( vertIndex2 < vertIndex0 ) {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex2, vertIndex0), (int)vertices.size() ) );
      } else {
        ret = edgeToVertexIndexMap.insert( std::pair< Edge, int >( Edge(vertIndex0, vertIndex2), (int)vertices.size() ) );
      }

      if (ret.second) {
        glm::vec3 vert5 = vertices[ vertIndex2 ] + vertices[ vertIndex0 ];
        vert5 = glm::normalize(vert5);
        vertIndex5 = (int)vertices.size();
        vertices.push_back( vert5 );
      } else {
        vertIndex5 = ret.first->second;
      }

      triangles[ triIndex ] = glm::ivec3( vertIndex0, vertIndex3, vertIndex5 );
      triangles.push_back( glm::ivec3( vertIndex3, vertIndex1, vertIndex4 ) );
      triangles.push_back( glm::ivec3( vertIndex3, vertIndex4, vertIndex5 ) );
      triangles.push_back( glm::ivec3( vertIndex4, vertIndex2, vertIndex5 ) );
    }
  }

  //dump to vertex stream
  using namespace mineola::vertex_type;
  std::shared_ptr<VertexStream> vs(new VertexStream);
  std::shared_ptr<VertexStream> ns(new VertexStream);
  std::shared_ptr<VertexStream> is(new VertexStream);
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = (uint32_t)vertices.size();
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, glm::value_ptr(vertices[0]));

  ns->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  ns->type = VST_VERTEX;
  ns->size = (uint32_t)vertices.size();
  ns->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER));
  ns->buffer_ptr->Bind();
  ns->buffer_ptr->SetData(ns->Stride() * ns->size, glm::value_ptr(vertices[0]));

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = (uint32_t)triangles.size() * 3;
  is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ELEMENT_ARRAY_BUFFER));
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, glm::value_ptr(triangles[0]));

  vertex_array.AddVertexStream(vs);
  vertex_array.AddVertexStream(ns);
  vertex_array.SetIndexStream(is);
}

void BuildCube(float size, vertex_type::VertexArray &vertex_array) {
  std::vector<float> vertices;
  std::vector<int> indices;

  static const float verts[] = {
    -1.f, 1.f, 1.f,
    1.f, 1.f, 1.f,
    1.f, -1.f, 1.f,
    -1.f, -1.f, 1.f,
    -1.f, 1.f, -1.f,
    1.f, 1.f, -1.f,
    1.f, -1.f, -1.f,
    -1.f, -1.f, -1.f
  };

  static const int inds[] = {
    0, 1, 2, 0, 2, 3,
    1, 5, 6, 1, 6, 2,
    4, 7, 6, 4, 6, 5,
    0, 3, 7, 0, 7, 4,
    4, 5, 1, 4, 1, 0,
    7, 3, 2, 7, 2, 6
  };
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 3; ++j)
      vertices.push_back(verts[3 * i + j] * size / 2.f);
    for (int j = 0; j < 3; ++j)
      vertices.push_back(verts[3 * i + j] / std::sqrt(3.f));
  }
  for (int i = 0; i < 36; ++i)
    indices.push_back(inds[i]);

  using namespace mineola::vertex_type;
  std::shared_ptr<VertexStream> vs(new VertexStream);
  std::shared_ptr<VertexStream> is(new VertexStream);
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = 8;
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, &vertices[0]);

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = 36;
  is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER));
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, &indices[0]);

  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}

void BuildFrustum(float size, float fov, float aspect_ratio,
  vertex_type::VertexArray &vertex_array) {

  static const float inv_sqrt2 = std::sqrt(1 / 2.f);
  float half_height = std::tan(fov / 2.f ) * size;
  float half_width = aspect_ratio * half_height;
  float verts[] = {
    0, 0, 0, /*position*/ 0, 0, 1, /*normal*/
    -half_width, -half_height, -size, -inv_sqrt2, -inv_sqrt2, 0,
    half_width, -half_height, -size, inv_sqrt2, -inv_sqrt2, 0,
    half_width, half_height, -size, inv_sqrt2, inv_sqrt2, 0,
    -half_width, half_height, -size, -inv_sqrt2, inv_sqrt2, 0
  };
  static const uint32_t inds[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 1,
  };

  using namespace mineola::vertex_type;
  auto vs = std::make_shared<VertexStream>();
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = 5;
  vs->buffer_ptr = std::make_shared<GraphicsBuffer>(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, verts);

  auto is = std::make_shared<VertexStream>();
  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = 12;
  is->buffer_ptr = std::make_shared<GraphicsBuffer>(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ELEMENT_ARRAY_BUFFER);
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, inds);

  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}

void BuildFrameAxes(float size, vertex_type::VertexArray &vertex_array) {
  float verts[] = {
    0, 0, 0, /*position*/ 1, 0, 0, /*color*/
    size, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0,
    0, size, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1,
    0, 0, size, 0, 0, 1
  };
  static const uint32_t inds[] = {
    0, 1,
    2, 3,
    4, 5
  };

  using namespace mineola::vertex_type;
  auto vs = std::make_shared<VertexStream>();
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->layout.push_back({DIFFUSE_COLOR, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = 6;
  vs->buffer_ptr = std::make_shared<GraphicsBuffer>(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, verts);

  auto is = std::make_shared<VertexStream>();
  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = 6;
  is->buffer_ptr = std::make_shared<GraphicsBuffer>(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ELEMENT_ARRAY_BUFFER);
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, inds);

  vertex_array.PrimitiveType() = GL_LINES;
  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}

void BuildMesh(const std::vector<glm::vec3> &positions,
  const std::vector<glm::ivec3> &faces,
  vertex_type::VertexArray &vertex_array) {

  using namespace mineola::vertex_type;
  std::shared_ptr<VertexStream> vs(new VertexStream);
  std::shared_ptr<VertexStream> is(new VertexStream);
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = (uint32_t)positions.size();
  vs->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ARRAY_BUFFER));
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, glm::value_ptr(positions[0]));

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = (uint32_t)faces.size() * 3;
  is->buffer_ptr.reset(new GraphicsBuffer(GraphicsBuffer::STATIC,
    GraphicsBuffer::SEND,
    GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER));
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, glm::value_ptr(faces[0]));

  vertex_array.AddVertexStream(vs);
  vertex_array.SetIndexStream(is);
}


}} //namespaces
