#ifndef MINEOLA_ENVBRDF_H
#define MINEOLA_ENVBRDF_H

#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <mineola/Imgpp.hpp>
#include <mineola/SH3.h>

namespace mineola {

namespace math {
  void SphToCart(float theta, float phi, glm::vec3 &cart);
  void CartToSph(const glm::vec3 &cart, float &theta, float &phi);
}  // namespace math

namespace utils {

  std::vector<imgpp::Img> BuildPyramid(imgpp::Img &img);

  glm::vec2 Hammersley(uint32_t i, uint32_t N);
  glm::mat3 TangentBasis(const glm::vec3 &z);
  glm::vec3 Tangent2World(const glm::vec3 &src, const glm::vec3 &z);

  float G_GGX(float roughness, float NoV, float NoL);
  float G_GGX_Approx(float roughness, float NoV, float NoL);

  glm::vec4 ImportanceSampleGGX(const glm::vec2 &Xi, float roughness);

  glm::vec2 IntegrateBRDF(float roughness, float NoV);

  glm::vec3 PrefilterSpecularEnvMap(float roughness, const glm::vec3 &R,
    const std::vector<imgpp::ImgROI> &pyramid);

}  // namespace utils
}  // namespaces mineola

#endif
