#ifndef MINEOLA_POLYGONSOUP_H
#define MINEOLA_POLYGONSOUP_H

#include <vector>
#include <string>
#include "GLMDefines.h"
#include <glm/glm.hpp>

namespace mineola {

  struct AABB;

  struct PolygonSoup {
    struct Vertex {
      glm::vec3 pos;
      glm::vec3 normal;
      glm::vec2 tex;
      union Color {
        uint32_t val{0};
        uint8_t rgba[4];
      } color;
    };

    struct Face {
      std::vector<size_t> indices;
      std::vector<glm::vec2> texcoords;
    };

    typedef std::pair<size_t, size_t> Edge;

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Edge> edges;

    bool has_vertex_normal{false};
    bool has_vertex_texcoord{false};
    bool has_vertex_color{false};
    bool has_face_texcoord{false};
    std::string texture_filename;

    void Clear();
    void ComputeVertexNormal();
    AABB ComputeAABB() const;
  };
}

#endif //MINEOLA_ENGINE_SIMPLEPLY_H
