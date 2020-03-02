#ifndef MINEOLA_LIGHT_H
#define MINEOLA_LIGHT_H

#include <memory>
#include <glm/glm.hpp>

namespace mineola {

namespace math { class Rbt; }

class Light {
public:
  Light(size_t idx);
  virtual ~Light();

  size_t Index() const;

  void SetProjParams(float fovy, float aspect, float near, float far);
  void SetOrthoProjParams(float left, float right, float bottom, float top, float near, float far);
  void SetProjMatrix(const glm::mat4 &proj_mat);
  void SetIntensity(glm::vec3 intensity);

  const glm::mat4 &GetLightViewMatrix() const;
  const glm::mat4 &GetLightProjMatrix() const;
  const glm::vec3 &GetPosition() const;
  const glm::vec3 &GetIntensity() const;
  void UpdateLightTransform(const math::Rbt &rbt);

protected:
  glm::mat4 view_mat_;
  glm::mat4 proj_mat_;
  glm::vec3 pos_;

  size_t idx_;
  glm::vec3 intensity_;
};

}

#endif
