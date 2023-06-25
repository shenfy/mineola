#ifndef MINEOLA_UNIFORMWRAPPERS_H
#define MINEOLA_UNIFORMWRAPPERS_H

#include <vector>
#include <memory>
#include "GLMDefines.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Material.h"
#include "GLEffect.h"

namespace mineola {

template<typename T>
class ScalarUniformWrapper : public UniformWrapper {
public:
  ScalarUniformWrapper(T value) : value_(std::move(value)) {}
  virtual void UploadToShader(const char *var_name, GLEffect *effect) override {
    effect->UploadVariable(var_name, &value_);
  }
private:
  T value_;
};

template<typename T>
class GLMUniformWrapper : public UniformWrapper {
public:
  GLMUniformWrapper(T value) : value_(std::move(value)) {}
  virtual void UploadToShader(const char *var_name, GLEffect *effect) override {
    effect->UploadVariable(var_name, glm::value_ptr(value_));
  }
private:
  T value_;
};

template<typename T>
class ScalarVectorUniformWrapper : public UniformWrapper {
public:
  ScalarVectorUniformWrapper(std::vector<T> value) : value_(std::move(value)) {}
  virtual void UploadToShader(const char *var_name, GLEffect *effect) override {
    effect->UploadVariable(var_name, &value_[0]);
  }
private:
  std::vector<T> value_;
};

template<typename T>
class GLMVectorUniformWrapper : public UniformWrapper {
public:
  GLMVectorUniformWrapper(std::vector<T> value) : value_(std::move(value)) {}
  virtual void UploadToShader(const char *var_name, GLEffect *effect) override {
    effect->UploadVariable(var_name, glm::value_ptr(value_[0]));
  }
private:
  std::vector<T> value_;
};

namespace uniform_helper {

std::shared_ptr<UniformWrapper> Wrap(int val);
std::shared_ptr<UniformWrapper> Wrap(uint32_t val);
std::shared_ptr<UniformWrapper> Wrap(float val);
std::shared_ptr<UniformWrapper> Wrap(glm::vec2 val);
std::shared_ptr<UniformWrapper> Wrap(glm::vec3 val);
std::shared_ptr<UniformWrapper> Wrap(glm::vec4 val);
std::shared_ptr<UniformWrapper> Wrap(glm::ivec2 val);
std::shared_ptr<UniformWrapper> Wrap(glm::ivec3 val);
std::shared_ptr<UniformWrapper> Wrap(glm::ivec4 val);
std::shared_ptr<UniformWrapper> Wrap(glm::uvec2 val);
std::shared_ptr<UniformWrapper> Wrap(glm::uvec3 val);
std::shared_ptr<UniformWrapper> Wrap(glm::uvec4 val);
std::shared_ptr<UniformWrapper> Wrap(glm::mat2 val);
std::shared_ptr<UniformWrapper> Wrap(glm::mat3 val);
std::shared_ptr<UniformWrapper> Wrap(glm::mat4 val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<int> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<uint32_t> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<float> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec2> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec3> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec4> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec2> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec3> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec4> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec2> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec3> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec4> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat2> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat3> val);
std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat4> val);

}  // namespace uniform_helper

}  // namespace mineola

#endif
