#ifndef IMGPP_GLMTRAITS_HPP
#define IMGPP_GLMTRAITS_HPP

#include "ImgppTypeTraits.hpp"
#include "GLMDefines.h"
#include <glm/glm.hpp>

namespace mineola {
namespace imgpp {

template<>
struct Interpolatable<glm::ivec2> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::ivec3> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::ivec4> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::uvec2> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::uvec3> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::uvec4> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::tvec2<uint8_t>> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::tvec3<uint8_t>> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::tvec4<uint8_t>> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::tvec2<uint16_t>> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::tvec3<uint16_t>> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::tvec4<uint16_t>> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::tvec2<uint64_t>> {
  using type = glm::dvec2;
};

template<>
struct Interpolatable<glm::tvec3<uint64_t>> {
  using type = glm::dvec3;
};

template<>
struct Interpolatable<glm::tvec4<uint64_t>> {
  using type = glm::dvec4;
};

template<>
struct Interpolatable<glm::tvec2<int8_t>> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::tvec3<int8_t>> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::tvec4<int8_t>> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::tvec2<int16_t>> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::tvec3<int16_t>> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::tvec4<int16_t>> {
  using type = glm::vec4;
};

template<>
struct Interpolatable<glm::tvec2<int64_t>> {
  using type = glm::dvec2;
};

template<>
struct Interpolatable<glm::tvec3<int64_t>> {
  using type = glm::dvec3;
};

template<>
struct Interpolatable<glm::tvec4<int64_t>> {
  using type = glm::dvec4;
};

template<>
struct Interpolatable<glm::bvec2> {
  using type = glm::vec2;
};

template<>
struct Interpolatable<glm::bvec3> {
  using type = glm::vec3;
};

template<>
struct Interpolatable<glm::bvec4> {
  using type = glm::vec4;
};


}}  // namespaces

#endif //IMGPP_GLMTRAITS_HPP
