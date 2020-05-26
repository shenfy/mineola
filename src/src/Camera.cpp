#include "prefix.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mineola/glutility.h>
#include <mineola/Camera.h>
#include <mineola/GLEffect.h>
#include <mineola/Engine.h>
#include <mineola/UniformBlock.h>
#include <mineola/Viewport.h>

namespace mineola {

Camera::Camera(bool perspective)
  : perspective_(perspective),
  fovy_(60.0f),
  near_(0.001f),
  far_(100.0f),
  aspect_ratio_(1.0f),
  left_(-1.0f),
  right_(1.0f),
  bottom_(-1.0f),
  top_(1.0f),
  using_custom_proj_matrix_(false) {
}

Camera::~Camera() {
}

void Camera::Activate() {
  auto builtin_ub = Engine::Instance().BuiltinUniformBlock().lock();
  if (builtin_ub) {
    builtin_ub->UpdateVariable("_view_mat", glm::value_ptr(view_mat_));
    builtin_ub->UpdateVariable("_view_mat_inv", glm::value_ptr(glm::inverse(view_mat_)));
    builtin_ub->UpdateVariable("_proj_mat", glm::value_ptr(proj_mat_));
    builtin_ub->UpdateVariable("_proj_mat_inv", glm::value_ptr(glm::inverse(proj_mat_)));
    builtin_ub->UpdateVariable("_proj_view_mat", glm::value_ptr(proj_mat_ * view_mat_));
  }
}

void Camera::SetProjParams(float fovy, float near_plane, float far_plane) {
  fovy_ = fovy;
  near_ = near_plane;
  far_ = far_plane;
  proj_mat_ = glm::perspective(fovy, aspect_ratio_, near_plane, far_plane);
}

void Camera::SetNearPlane(float near_plane) {
  SetProjParams(fovy_, near_plane, far_);
}

float Camera::NearPlane() const {
  return near_;
}

void Camera::SetFarPlane(float far_plane) {
  SetProjParams(fovy_, near_, far_plane);
}

float Camera::FarPlane() const {
  return far_;
}

void Camera::SetFov(float fov) {
  SetProjParams(fov, near_, far_);
}

float Camera::Fov() const {
  return fovy_;
}

void Camera::SetOrthoProjParams(float left, float right,
  float bottom, float top,
  float near_plane, float far_plane) {
  left_ = left;
  right_ = right;
  bottom_ = bottom;
  top_ = top;
  near_ = near_plane;
  far_ = far_plane;
}


void Camera::SetViewMatrix(const glm::mat4 &view_mat) {
  view_mat_ = view_mat;
}

void Camera::SetProjMatrix(const glm::mat4 &proj_mat) {
  proj_mat_ = proj_mat;
  using_custom_proj_matrix_ = true;
}

void Camera::OnSize(const Viewport *viewport) {
  if (viewport && viewport->height != 0) {
    if (!using_custom_proj_matrix_) {
      aspect_ratio_ = (float)viewport->width / viewport->height;
      if (perspective_) {
        proj_mat_ = glm::perspective(fovy_, aspect_ratio_, near_, far_);
      } else {
        float center = (bottom_ + top_) * .5f;
        float diff = (top_ - center) / aspect_ratio_;
        proj_mat_ = glm::ortho(left_, right_, center - diff, center + diff, near_, far_);
      }
    }
  }
}

Resizer::Resizer(uint32_t width, uint32_t height)
  : width_(width), height_(height) {
}

Resizer::~Resizer() {
}

}
