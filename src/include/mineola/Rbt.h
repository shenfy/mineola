#ifndef MINEOLA_RBT_HPP
#define MINEOLA_RBT_HPP

#include "GLMDefines.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mineola { namespace math {

class Rbt {
public:

  // constructors and destructors
  Rbt();
  Rbt(const glm::vec3 &t, const glm::quat &r);
  Rbt(const glm::vec3 &t);
  Rbt(const glm::quat &r);
  ~Rbt();

  // accessers
  glm::vec3 &Translation();
  const glm::vec3 &Translation() const;
  glm::quat &Rotation();
  const glm::quat &Rotation() const;

  // math
  glm::vec4 operator* (const glm::vec4 &v) const;
  Rbt operator* (const Rbt &other) const;
  Rbt TranslationRbt() const;
  Rbt RotationRbt() const;
  glm::mat4 ToMatrix() const;

  static Rbt Inv(const Rbt &rbt);
  static Rbt Lerp(const Rbt &rbt0, const Rbt &rbt1, float t);
  static Rbt DoMToOWrtA(const Rbt &m, const Rbt &o, const Rbt &a);
  static Rbt LookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up);
  static Rbt CatmullRom(
    const Rbt &rbt0, const Rbt &rbt1, const Rbt &rbt2, const Rbt &rbt3, float t);

private:
  glm::vec3 t_;
  glm::quat r_;

};

}}  // namespaces

#endif