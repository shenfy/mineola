#include "prefix.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/Light.h>
#include <mineola/Rbt.h>
#include <mineola/UniformBlock.h>

namespace mineola {

Light::Light(size_t idx)
  : idx_(idx) {
}

Light::~Light() = default;

size_t Light::Index() const {
  return idx_;
}

PointDirLight::PointDirLight(size_t idx) : Light(idx) {
}

PointDirLight::~PointDirLight() = default;

void PointDirLight::SetProjParams(float fovy, float aspect, float near_plane, float far_plane) {
  proj_mat_ = glm::perspective(fovy, aspect, near_plane, far_plane);
}

void PointDirLight::SetOrthoProjParams(float left, float right, float bottom, float top,
  float z_near, float z_far) {
  proj_mat_ = glm::ortho(left, right, bottom, top, z_near, z_far);
}

void PointDirLight::SetProjMatrix(const glm::mat4 &proj_mat) {
  proj_mat_ = proj_mat;
}

void PointDirLight::SetIntensity(glm::vec3 intensity) {
  intensity_ = std::move(intensity);
}

void PointDirLight::UpdateUniforms(UniformBlock *ub) {
  auto v4 = glm::vec4(pos_, 1.0f);
  auto idx_str = std::to_string(idx_);
  auto var_name = "_light_pos_" + idx_str;
  ub->UpdateVariable(var_name.c_str(), glm::value_ptr(v4));

  v4 = glm::vec4(intensity_, 1.0f);
  var_name = "_light_intensity_" + idx_str;
  ub->UpdateVariable(var_name.c_str(), glm::value_ptr(v4));

  var_name = "_light_view_mat_" + idx_str;
  ub->UpdateVariable(var_name.c_str(), glm::value_ptr(view_mat_));

  var_name = "_light_proj_mat_" + idx_str;
  ub->UpdateVariable(var_name.c_str(), glm::value_ptr(proj_mat_));
}

void PointDirLight::UpdateLightTransform(const math::Rbt &rbt) {
  view_mat_ = glm::inverse(rbt.ToMatrix());
  pos_ = rbt.Translation();
}

}  // end namespace
