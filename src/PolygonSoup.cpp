#include "prefix.h"
#include "../include/PolygonSoup.h"

namespace mineola {

  void PolygonSoup::Clear() {
    vertices.clear();
    faces.clear();
    edges.clear();
    texture_filename.clear();
    has_vertex_normal = false;
    has_vertex_texcoord = false;
    has_face_texcoord = false;
    has_vertex_color = false;
  }

  void PolygonSoup::ComputeVertexNormal() {
    std::vector<std::vector<glm::vec3>> face_normals(vertices.size());
    for (const auto &face : faces) {
      if (face.indices.size() < 3)
        continue;
      auto edge1 = vertices[face.indices[1]].pos - vertices[face.indices[0]].pos;
      auto edge2 = vertices[face.indices[2]].pos - vertices[face.indices[1]].pos;
      auto normal = glm::normalize(glm::cross(edge1, edge2));
      if (glm::isnan(normal.x) || glm::isnan(normal.y) || glm::isnan(normal.z))
        continue;
      for (auto index : face.indices)
        face_normals[index].push_back(normal);
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
      if (face_normals[i].size() > 0) {
        auto &normal = vertices[i].normal;
        normal = {0, 0, 0};
        for (auto &n : face_normals[i])
          normal = normal + n;
        normal = normal / glm::vec3::value_type(face_normals.size());
        normal = glm::normalize(normal);
        if (glm::isnan(normal.x) || glm::isnan(normal.y) || glm::isnan(normal.z))
          normal = {0, 0, 0};
      }
    }
  }
}
