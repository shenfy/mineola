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

class PointLight : public Light {
public:
  PointLight(size_t idx);
  ~PointLight() override;

  void UpdateLightTransform(const math::Rbt &rbt) override;
  void UpdateUniforms(UniformBlock *ub) override;

  void SetProjParams(float fovy, float aspect, float near, float far);
  void SetProjMatrix(const glm::mat4 &proj_mat);
  void SetIntensity(glm::vec3 intensity);

protected:
  glm::mat4 view_mat_;
  glm::mat4 proj_mat_;
  glm::vec3 pos_;
  glm::vec3 intensity_;

  void UploadUniforms(glm::vec4 &v4, UniformBlock *ub);
};

class DirLight : public PointLight {
public:
  DirLight(size_t idx);
  ~DirLight() override;
  
  void SetOrthoProjParams(float left, float right, 
    float bottom, float top,
    float near, float far);
  void UpdateUniforms(UniformBlock *ub) override;
};

}

#endif
