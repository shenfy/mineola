#ifndef MINEOLA_SH3_HPP
#define MINEOLA_SH3_HPP

#include <vector>

namespace mineola { namespace math {

inline void SH3Basis(const glm::vec3 &n, std::vector<float> &basis) {
  basis[0] = 0.282095f;

  basis[1] = 0.488603f * n.y;
  basis[2] = 0.488603f * n.z;
  basis[3] = 0.488603f * n.x;

  basis[4] = 1.092548f * n.x * n.y;
  basis[5] = 1.092548f * n.y * n.z;
  basis[6] = 0.315392f * (3.f * n.z * n.z - 1.f);
  basis[7] = 1.092548f * n.x * n.z;
  basis[8] = 0.546274f * (n.x * n.x - n.y * n.y);
}

template<typename T = float>
struct SH3 {
  enum {kNumCoeffs = 9};

  std::vector<T> coeffs;

  SH3();

  T Eval(const glm::vec3 &normal) const;

  T EvalCosine(const glm::vec3 &normal) const;
};

template<typename T>
SH3<T>::SH3() : coeffs(kNumCoeffs) {}

template<typename T>
T SH3<T>::Eval(const glm::vec3 &n) const {
  T result = coeffs[0] * 0.282095f;

  result += coeffs[1] * (0.488603f * n.y);
  result += coeffs[2] * (0.488603f * n.z);
  result += coeffs[3] * (0.488603f * n.x);

  result += coeffs[4] * (1.092548f * n.x * n.y);
  result += coeffs[5] * (1.092548f * n.y * n.z);
  result += coeffs[6] * (0.315392f * (3.f * n.z * n.z - 1.f));
  result += coeffs[7] * (1.092548f * n.x * n.z);
  result += coeffs[8] * (0.546274f * (n.x * n.x - n.y * n.y));

  return result;
}

template<typename T>
T SH3<T>::EvalCosine(const glm::vec3 &n) const {
  T result = coeffs[0] * 0.886227f;

  result += coeffs[1] * (2.f * 0.511664f * n.y); // ( 2 * π / 3 ) * 0.488603
  result += coeffs[2] * (2.f * 0.511664f * n.z);
  result += coeffs[3] * (2.f * 0.511664f * n.x);

  result += coeffs[4] * (2.f * 0.429043f * n.x * n.y); // ( π / 4 ) * 1.092548
  result += coeffs[5] * (2.f * 0.429043f * n.y * n.z);
  result += coeffs[6] * (0.743125f * n.z * n.z - 0.247708f); // ( π / 4 ) * 0.315392 * 3
  result += coeffs[7] * (2.f * 0.429043f * n.x * n.z);
  result += coeffs[8] * (0.429043f * (n.x * n.x - n.y * n.y)); // ( π / 4 ) * 0.546274

  return result;
}

}}  // namespaces

#endif
