#include "prefix.h"
#include "../include/UniformWrappers.h"
#include "../include/glutility.h"

namespace mineola { namespace uniform_helper {

std::shared_ptr<UniformWrapper> Wrap(int val) {
  return std::make_shared<ScalarUniformWrapper<int>>(val);
}

std::shared_ptr<UniformWrapper> Wrap(uint32_t val) {
  return std::make_shared<ScalarUniformWrapper<uint32_t>>(val);
}

std::shared_ptr<UniformWrapper> Wrap(float val) {
  return std::make_shared<ScalarUniformWrapper<float>>(val);
}

std::shared_ptr<UniformWrapper> Wrap(glm::vec2 val) {
  return std::make_shared<GLMUniformWrapper<glm::vec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::vec3 val) {
  return std::make_shared<GLMUniformWrapper<glm::vec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::vec4 val) {
  return std::make_shared<GLMUniformWrapper<glm::vec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::ivec2 val) {
  return std::make_shared<GLMUniformWrapper<glm::ivec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::ivec3 val) {
  return std::make_shared<GLMUniformWrapper<glm::ivec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::ivec4 val) {
  return std::make_shared<GLMUniformWrapper<glm::ivec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::uvec2 val) {
  return std::make_shared<GLMUniformWrapper<glm::uvec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::uvec3 val) {
  return std::make_shared<GLMUniformWrapper<glm::uvec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::uvec4 val) {
  return std::make_shared<GLMUniformWrapper<glm::uvec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::mat2 val) {
  return std::make_shared<GLMUniformWrapper<glm::mat2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::mat3 val) {
  return std::make_shared<GLMUniformWrapper<glm::mat3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(glm::mat4 val) {
  return std::make_shared<GLMUniformWrapper<glm::mat4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<int> val) {
  return std::make_shared<ScalarVectorUniformWrapper<int>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<uint32_t> val) {
  return std::make_shared<ScalarVectorUniformWrapper<uint32_t>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<float> val) {
  return std::make_shared<ScalarVectorUniformWrapper<float>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec2> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::vec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec3> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::vec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::vec4> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::vec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec2> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::ivec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec3> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::ivec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::ivec4> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::ivec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec2> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::uvec2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec3> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::uvec3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::uvec4> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::uvec4>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat2> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::mat2>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat3> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::mat3>>(std::move(val));
}

std::shared_ptr<UniformWrapper> Wrap(std::vector<glm::mat4> val) {
  return std::make_shared<GLMVectorUniformWrapper<glm::mat4>>(std::move(val));
}

}} //namespace
