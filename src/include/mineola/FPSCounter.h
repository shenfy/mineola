#ifndef MINEOLA_FPSCOUNTER_H
#define MINEOLA_FPSCOUNTER_H

#include <boost/circular_buffer.hpp>
#include <mineola/Entity.h>

namespace mineola {

class TextLabel;

class FPSCounter : public Entity {
public:
  FPSCounter();
  FPSCounter(size_t num_frames);
  ~FPSCounter() override;

  bool CreateLabel(const char *font_name);

  void FrameMove(double time, double frame_time) override;

protected:
  size_t num_frames_;
  boost::circular_buffer<double> time_stamps_;
  std::weak_ptr<TextLabel> label_;

};

} //namespaces

#endif