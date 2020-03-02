#include "prefix.h"
#include "../include/Light.h"
#include "../include/Rbt.h"
#include <glm/gtc/matrix_transform.hpp>

namespace mineola {

Light::Light(size_t idx)
  : idx_(idx) {
}

Light::~Light() {
}

void Light::SetProjParams(float fovy, float aspect, float near_plane, float far_plane) {
  proj_mat_ = glm::perspective(fovy, aspect, near_plane, far_plane);
}

void Light::SetOrthoProjParams(float left, float right, float bottom, float top,
  float z_near, float z_far) {
  proj_mat_ = glm::ortho(left, right, bottom, top, z_near, z_far);
}

void Light::SetProjMatrix(const glm::mat4 &proj_mat) {
  proj_mat_ = proj_mat;
}

void Light::SetIntensity(glm::vec3 intensity) {
  intensity_ = std::move(intensity);
}

size_t Light::Index() const {
  return idx_;
}

const glm::mat4 &Light::GetLightViewMatrix() const {
  return view_mat_;
}

const glm::mat4 &Light::GetLightProjMatrix() const {
  return proj_mat_;
}

const glm::vec3 &Light::GetPosition() const {
  return pos_;
}

const glm::vec3 &Light::GetIntensity() const {
  return intensity_;
}

void Light::UpdateLightTransform(const math::Rbt &rbt) {
  view_mat_ = glm::inverse(rbt.ToMatrix());
  pos_ = rbt.Translation();
}

}  // end namespace