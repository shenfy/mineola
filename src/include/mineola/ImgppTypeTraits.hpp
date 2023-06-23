#ifndef IMGPP_TYPETRAITS_HPP
#define IMGPP_TYPETRAITS_HPP

#include <cstdint>

namespace mineola {
namespace imgpp {

template<typename T>
struct Interpolatable {
  using type = T;
};

template<>
struct Interpolatable<uint8_t> {
  using type = float;
};

template<>
struct Interpolatable<int8_t> {
  using type = float;
};

template<>
struct Interpolatable<uint16_t> {
  using type = float;
};

template<>
struct Interpolatable<int16_t> {
  using type = float;
};

template<>
struct Interpolatable<uint32_t> {
  using type = float;
};

template<>
struct Interpolatable<int32_t> {
  using type = float;
};

template<>
struct Interpolatable<uint64_t> {
  using type = double;
};

template<>
struct Interpolatable<int64_t> {
  using type = double;
};

}} //namespaces

#endif //IMGPP_TYPETRAITS_HPP
