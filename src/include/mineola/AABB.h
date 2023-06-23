#ifndef MINEOLA_AABB_H
#define MINEOLA_AABB_H

#include <memory>
#include "GLMDefines.h"
#include <glm/glm.hpp>

namespace mineola {
struct AABB {
public:
  AABB(const glm::vec3 &lb, const glm::vec3 &ub);
  glm::vec3 Center() const;
  glm::vec3 Extent() const;
  std::vector<glm::vec3> Corners() const;
  AABB &Combine(const AABB &other);
  AABB &Transform(const glm::mat4 &mat);
public:
  glm::vec3 lb_{0.f, 0.f, 0.f};
  glm::vec3 ub_{0.f, 0.f, 0.f};
};
} // namespace

#endif
