#include "prefix.h"
#include <mineola/FPSCounter.h>
#include <mineola/Engine.h>
#include <mineola/TextLabel.h>
#include <mineola/SceneNode.h>

namespace mineola {

FPSCounter::FPSCounter()
  : num_frames_(10), time_stamps_(num_frames_) {
}

FPSCounter::FPSCounter(size_t num_frames)
  : num_frames_(num_frames), time_stamps_(num_frames_) {
}

FPSCounter::~FPSCounter() {
}

void FPSCounter::FrameMove(double time, double frame_time) {
  static char buffer[32] = {0};

  time_stamps_.push_back(time);

  auto label = label_.lock();
  if (label) {
    auto delta_time = time_stamps_.back() - time_stamps_.front();
    auto fps = (time_stamps_.size() - 1) / delta_time * 1000.0;
    std::sprintf(buffer, "FPS: %.1f", fps);
    label->SetText(buffer);
  }

}

bool FPSCounter::CreateLabel(const char *font_name) {
  auto label = std::make_shared<TextLabel>(16, font_name);
  if (!label) {
    return false;
  }
  auto &en = Engine::Instance();
  en.Scene()->Renderables().push_back(bd_cast<Renderable>(label));
  label_ = label;
  return true;
}

}