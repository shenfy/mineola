#ifndef MINEOLA_PRIMITIVE_HELPER
#define MINEOLA_PRIMITIVE_HELPER
#include "VertexType.h"
#include <glm/glm.hpp>

namespace mineola { namespace primitive_helper {

void BuildRect(float size, vertex_type::VertexArray &vertex_array);
void BuildRectXY(float size, vertex_type::VertexArray &vertex_array);
void BuildSphere(int subDivLevel, vertex_type::VertexArray &vertex_array);
void BuildCube(float size, vertex_type::VertexArray &vertex_array);
void BuildFrustum(float size, float fov, float aspect_ratio,
  vertex_type::VertexArray &vertex_array
);
void BuildFrameAxes(float size, vertex_type::VertexArray &vertex_array);

void BuildMesh(const std::vector<glm::vec3> &pts, const std::vector<glm::ivec3> &faces,
    vertex_type::VertexArray &vertex_array);

}}

#endif
