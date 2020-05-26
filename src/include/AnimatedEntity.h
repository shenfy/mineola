#ifndef MINEOLA_ANIMATEDENTITY_H
#define MINEOLA_ANIMATEDENTITY_H

#include "Entity.h"
#include "Animation.h"

namespace mineola {

class AnimatedEntity : public Entity {
public:
  AnimatedEntity();
  virtual ~AnimatedEntity();

  // life cycle
  void Start() override;
  void FrameMove(double time, double frame_time) override;
  void PreRender() override;
  void PostRender() override;
  void Destroy() override;

  // add animation
  void AddAnimation(animation::Animation &&animation);

  enum {kPlayOnce = 0, kPlayLoop};
  void SetPlayMode(int play_mode);
  void SetSpeed(double speed);

  // messages to trigger state change
  enum {kIdle = 0, kPlaying, kPaused, kSnapshot, kReset};
  void Play();
  void Pause();
  void Reset();
  void Snapshot(double offset);

protected:
  int state_{kIdle};
  int play_mode_{kPlayOnce};
  double start_time_{0.0};
  double start_offset_{0.0};
  double speed_{1.0};
  double length_{0.0};
  // todo: evolve into a Unity like complex state machine
  std::vector<animation::Animation> animations_;
};

} //namespace

#endif