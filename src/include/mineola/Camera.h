#ifndef MINEOLA_CAMERA_H
#define MINEOLA_CAMERA_H

#include <glm/glm.hpp>

namespace mineola {

class GLEffect;
class Viewport;

class Camera {
public:
  explicit Camera(bool perspective = true);
  virtual ~Camera();

  void SetProjParams(float fovy, float near, float far);
  void SetNearPlane(float near);
  float NearPlane() const;
  void SetFarPlane(float far);
  float FarPlane() const;
  void SetFov(float fov);
  float Fov() const;
  void SetOrthoProjParams(float left, float right, float bottom, float top, float near, float far);
  void SetViewMatrix(const glm::mat4 &view_mat);
  void SetProjMatrix(const glm::mat4 &proj_mat);
  const glm::mat4 &GetViewMatrix() const {return view_mat_;}
  const glm::mat4 &GetProjMatrix() const {return proj_mat_;}
  void OnSize(const Viewport *viewport);

  virtual void Activate();

  // MINEOLA_VISITOR_ACCEPT_FUNC

protected:
  glm::mat4 view_mat_;

  bool perspective_;
  //for perspective projection
  float fovy_, near_, far_;
  float aspect_ratio_;
  //for orthogonal projection
  float left_, right_, bottom_, top_;
  glm::mat4 proj_mat_;

  // record if projection matrix is directly set
  bool using_custom_proj_matrix_;
};
}

#endif
