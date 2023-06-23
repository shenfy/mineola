#ifndef MINEOLA_MATHHELPER_H
#define MINEOLA_MATHHELPER_H

#include "GLMDefines.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace mineola { namespace math {

  inline glm::vec4 operator*(const glm::quat &q, const glm::vec4 &v) {
    glm::quat tmp(0.0f, v[0], v[1], v[2]);
    tmp = glm::cross(glm::cross(q, tmp), glm::inverse(q));
    return glm::vec4(tmp[0], tmp[1], tmp[2], v[3]);
  }

  inline glm::quat ShortRotation(const glm::quat &q) {
    return q.w < 0 ? -q : q;
  }

  inline glm::mat4 DoMToOWrtA(const glm::mat4 &m, const glm::mat4 &o, const glm::mat4 &a) {
    return a * m * glm::inverse(a) * o;
  }

  template<typename T>
  T CatmullRomVec(const T &v0, const T &v1, const T &v2, const T &v3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float s = 1 - t;
    float s2 = s * s;
    float s3 = s2 * s;
    T i1 = v1 + (v2 - v0) * (1.0f / 6.0f);
    T i2 = v2 - (v3 - v1) * (1.0f / 6.0f);
    return v1 * s3 + i1 * (3 * s2 * t) + i2 * (3 * s * t2) + v2 * t3;
  }

  template<typename T>
  glm::tquat<T> CatmullRom(
    const glm::tquat<T> &q0, const glm::tquat<T> &q1,
    const glm::tquat<T> &q2, const glm::tquat<T> &q3,
    float t) {

    glm::tquat<T> i1 = glm::pow(
      ShortRotation(q2 * glm::inverse(q0)), 1.0f / 6.0f) * q1;
    glm::tquat<T> i2 = glm::inverse(glm::pow(
      ShortRotation(q3 * glm::inverse(q1)), 1.0f / 6.0f)) * q2;
    glm::tquat<T> p01 = glm::slerp(q1, i1, t);
    glm::tquat<T> p12 = glm::slerp(i1, i2, t);
    glm::tquat<T> p23 = glm::slerp(i2, q2, t);
    // 3rd order bezier interpolation
    return glm::slerp(glm::slerp(p01, p12, t), glm::slerp(p12, p23, t), t);
  }

}}

#endif
