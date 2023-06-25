#ifndef MINEOLA_LIGHT_H
#define MINEOLA_LIGHT_H

#include <memory>
#include "GLMDefines.h"
#include <glm/glm.hpp>

namespace mineola {

namespace math { class Rbt; }

class UniformBlock;

class Light {
public:
  Light(size_t idx);
  virtual ~Light();

  size_t Index() const;

  virtual void UpdateLightTransform(const math::Rbt &rbt) = 0;

  virtual void UpdateUniforms(UniformBlock *ub) = 0;

protected:
  size_t idx_;
};

class PointDirLight : public Light {
public:
  PointDirLight(size_t idx);
  virtual ~PointDirLight() override;

  void UpdateLightTransform(const math::Rbt &rbt) override;
  void UpdateUniforms(UniformBlock *ub) override;

  void SetProjParams(float fovy, float aspect, float near, float far);
  void SetOrthoProjParams(float left, float right, float bottom, float top, float near, float far);
  void SetProjMatrix(const glm::mat4 &proj_mat);
  void SetIntensity(glm::vec3 intensity);

protected:
  glm::mat4 view_mat_;
  glm::mat4 proj_mat_;
  glm::vec3 pos_;
  glm::vec3 intensity_;
};


}

#endif
