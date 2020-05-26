#include "prefix.h"
#include <mineola/Rbt.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mineola/MathHelper.h>

namespace mineola { namespace math {

Rbt::Rbt() {}

Rbt::Rbt(const glm::vec3 &t, const glm::quat &r) : t_(t), r_(r) {
}

Rbt::Rbt(const glm::vec3 &t) : t_(t) {
}

Rbt::Rbt(const glm::quat &r) : r_(r) {
}

Rbt::~Rbt() {
}

glm::vec3 &Rbt::Translation() {
  return t_;
}

const glm::vec3& Rbt::Translation() const {
  return t_;
}

const glm::quat& Rbt::Rotation() const {
  return r_;
}

glm::quat &Rbt::Rotation() {
  return r_;
}

glm::vec4 Rbt::operator*(const glm::vec4 &v) const {
  return glm::vec4(t_, 0.0f) * v[3] + r_ * v;
}

Rbt Rbt::operator*(const Rbt &other) const {
  return Rbt(
    t_ + glm::vec3(r_ * glm::vec4(other.t_, 0.0f)),
    glm::cross(r_, other.r_));
}

Rbt Rbt::TranslationRbt() const {
  return Rbt(t_);
}

Rbt Rbt::RotationRbt() const {
  return Rbt(r_);
}

Rbt Rbt::Inv(const Rbt &rbt) {
  glm::quat r_inv = glm::inverse(rbt.r_);
  glm::vec4 tmp = r_inv * glm::vec4(-rbt.t_, 1.0f);
  return Rbt(glm::vec3(tmp), r_inv);
}

glm::mat4 Rbt::ToMatrix() const {
  glm::mat4 m = glm::mat4_cast(r_);
  for (int i = 0; i < 3; ++i)
    m[3][i] = t_[i];
  return m;
}

Rbt Rbt::Lerp(const Rbt &rbt0, const Rbt &rbt1, float t) {
  return Rbt(glm::mix(rbt0.t_, rbt1.t_, t),
    glm::slerp(rbt0.r_, rbt1.r_, t));
}

Rbt Rbt::DoMToOWrtA(const Rbt &m, const Rbt &o, const Rbt &a) {
  return a * m * Rbt::Inv(a) * o;
}

Rbt Rbt::LookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up) {
  glm::mat4 mat = glm::inverse(glm::lookAt(eye, target, up));
  glm::quat r = glm::quat_cast(mat);
  return Rbt(eye, r);
}

Rbt Rbt::CatmullRom(
  const Rbt &rbt0, const Rbt &rbt1, const Rbt &rbt2, const Rbt &rbt3, float t) {

  return Rbt(math::CatmullRomVec(rbt0.t_, rbt1.t_, rbt2.t_, rbt3.t_, t),
    math::CatmullRom(rbt0.r_, rbt1.r_, rbt2.r_, rbt3.r_, t));
}


}}
