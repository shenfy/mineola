#include "prefix.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/AABB.h>

namespace {
float BoolToSign(bool b) {
  return b ? 1.f : -1.f;
}
}

namespace mineola {

AABB::AABB(const glm::vec3 &lb, const glm::vec3 &ub) : lb_(lb), ub_(ub) {}

glm::vec3 AABB::Center() const {
  return (lb_ + ub_) * 0.5f;
}

glm::vec3 AABB::Extent() const {
  return ub_ - lb_;
}

std::vector<glm::vec3> AABB::Corners() const {
  auto center = Center();
  auto extent = Extent();
  auto corners = std::vector<glm::vec3>(8, glm::vec3());

  for (int i = 0; i < corners.size(); i++) {
    glm::vec3 sign(BoolToSign(i & 1), BoolToSign(i & 2), BoolToSign(i & 4));
    corners[i] = center + extent * 0.5f * sign;
  }
  return corners;
}

AABB &AABB::Combine(const AABB &other) {
  lb_ = glm::min(lb_, other.lb_);
  ub_ = glm::max(ub_, other.ub_);
  return *this;
}

AABB &AABB::Transform(const glm::mat4 &mat) {
  auto lb = glm::vec3(std::numeric_limits<float>::max());
  auto ub = glm::vec3(std::numeric_limits<float>::lowest());

  auto corners = Corners();
  for(auto &corner : corners) {
    corner = mat * glm::vec4(corner, 1.f);
    lb = glm::min(lb, corner);
    ub = glm::max(ub, corner);
  }
  lb_ = lb;
  ub_ = ub;
  return *this;
}
}
