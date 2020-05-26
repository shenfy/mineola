#ifndef MINEOLA_NONCOPYABLE_H
#define MINEOLA_NONCOPYABLE_H

namespace mineola {

class Noncopyable {
protected:
  Noncopyable() = default;
  ~Noncopyable() = default;

  Noncopyable(const Noncopyable&) = delete;
  Noncopyable& operator=(const Noncopyable &) = delete;
};

}

#endif
