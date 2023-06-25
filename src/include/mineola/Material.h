#ifndef MINEOLA_MATERIAL_H
#define MINEOLA_MATERIAL_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "GLMDefines.h"
#include <glm/glm.hpp>
#include "BasisObj.h"

namespace mineola {

class GLEffect;

class UniformWrapper {
public:
  virtual void UploadToShader(const char *var_name, GLEffect *effect) = 0;
};

struct Material : public Resource {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emit;
  float alpha{1.0f};
  float specularity{0.0f};
  float roughness{0.0f};

  std::unordered_map<std::string, std::vector<std::string>> texture_slots;
  std::unordered_map<std::string, std::shared_ptr<UniformWrapper>> uniform_slots;

  virtual void UploadToShader(GLEffect *effect);
};

}

#endif