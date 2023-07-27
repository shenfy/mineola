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

PointLight::PointLight(size_t idx) : Light(idx) {
}

PointLight::~PointLight() = default;

void PointLight::SetProjParams(float fovy, float aspect, 
  float near_plane, float far_plane) {
  proj_mat_ = glm::perspective(fovy, aspect, near_plane, far_plane);
}

void PointLight::SetProjMatrix(const glm::mat4 &proj_mat) {
  proj_mat_ = proj_mat;
}

void PointLight::SetIntensity(glm::vec3 intensity) {
  intensity_ = std::move(intensity);
}

void PointLight::UploadUniforms(glm::vec4 &v4, UniformBlock *ub) {
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

void PointLight::UpdateUniforms(UniformBlock *ub) {
  auto v4 = glm::vec4(pos_, 1.0f);
  UploadUniforms(v4, ub);
}

void PointLight::UpdateLightTransform(const math::Rbt &rbt) {
  view_mat_ = glm::inverse(rbt.ToMatrix());
  pos_ = rbt.Translation();
}

DirLight::DirLight(size_t idx) : PointLight(idx) {
}

DirLight::~DirLight() = default;

void DirLight::SetOrthoProjParams(float left, float right, 
  float bottom, float top,
  float z_near, float z_far) {
  proj_mat_ = glm::ortho(left, right, bottom, top, z_near, z_far);
}

void DirLight::UpdateUniforms(UniformBlock *ub) {
  auto v4 = glm::vec4(pos_, 0.0f);
  UploadUniforms(v4, ub);
}

}  // end namespace
