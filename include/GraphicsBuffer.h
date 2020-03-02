#ifndef MINEOLA_GRAPHICSBUFFER_H
#define MINEOLA_GRAPHICSBUFFER_H
#include <cstdint>
#include <vector>
#include <boost/noncopyable.hpp>

namespace mineola {

class GraphicsBuffer : private boost::noncopyable {
public:
  GraphicsBuffer(uint32_t frequency, uint32_t direction, uint32_t access, uint32_t target);
  GraphicsBuffer(uint32_t frequency, uint32_t direction, uint32_t access,
    std::vector<uint32_t> targets);
  GraphicsBuffer(uint32_t frequency, uint32_t direction, uint32_t access,
    uint32_t target, uint32_t index);
  ~GraphicsBuffer();

  uint32_t Handle();
  void SetBindTargets(std::vector<uint32_t> targets); //P.S. Call after unbinding buffer!
  void Bind();
  void BindBase();
  void Unbind();
  bool SetSize(uint32_t size);
  bool SetData(uint32_t size, const void *data);
  bool UpdateData(uint32_t offset, uint32_t size, const void *data);

  enum Frequency { STATIC = 0, DYNAMIC = 1, STREAM =2 };
  enum Direction { SEND = 0, READ = 1, COPY = 2 };
  enum Access { READ_ONLY = 0, WRITE_ONLY = 1, READ_WRITE = 2 };

protected:
  uint32_t buffer_handle_;
  uint32_t frequency_;
  uint32_t direction_;
  uint32_t access_;
  std::vector<uint32_t> targets_;
  uint32_t index_;
};

}

#endif
