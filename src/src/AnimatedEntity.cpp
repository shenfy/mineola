#include "prefix.h"
#include "../include/AnimatedEntity.h"
#include "../include/Engine.h"

namespace mineola {

AnimatedEntity::AnimatedEntity() {
}

AnimatedEntity::~AnimatedEntity() {
}

void AnimatedEntity::AddAnimation(animation::Animation &&animation) {
  auto length = animation.Length();
  if (length > length_) {
    length_ = length;
  }
  animations_.push_back(std::move(animation));
}

void AnimatedEntity::SetPlayMode(int play_mode) {
  play_mode_ = play_mode;
}

void AnimatedEntity::SetSpeed(double speed) {
  speed_ = std::max(0.0, speed);
}

void AnimatedEntity::Play() {
  auto &en = Engine::Instance();

  switch (state_) {
    case kIdle: {  // start
      start_time_ = en.Now();
      start_offset_ = 0.0;
      state_ = kPlaying;
      break;
    }
    case kPaused:  // fall through
    case kSnapshot: {  // resume
      start_time_ = en.Now();
      state_ = kPlaying;
      break;
    }
    default: {
      break;
    }
  }
}

void AnimatedEntity::Pause() {
  auto &en = Engine::Instance();
  switch (state_) {
    case kPlaying: {
      start_offset_ += en.Now() - start_time_;
      state_ = kPaused;
      break;
    }
    default: {
      break;
    }
  }
}

void AnimatedEntity::Reset() {
  switch (state_) {
    case kPlaying:
    case kPaused:
    case kSnapshot: {
      start_time_ = 0.0;
      start_offset_ = 0.0;
      state_ = kReset;
      break;
    }
    default: {
      break;
    }
  }
}

void AnimatedEntity::Snapshot(double offset) {
  auto &en = Engine::Instance();
  start_time_ = en.Now();
  start_offset_ = offset;
  state_ = kSnapshot;
}

void AnimatedEntity::Start() {}

void AnimatedEntity::FrameMove(double time, double frame_time) {
  if (state_ == kPlaying) {
    double offset = time - start_time_ + start_offset_;
    offset *= speed_;
    if (offset > length_) {
      if (play_mode_ == kPlayOnce) {  // stop animating
        Reset();
        return;
      } else if (play_mode_ == kPlayLoop) {  // return to beginning
        offset = std::fmod(offset, length_);
      }
    }

    for (auto &anim : animations_) {
      anim.Apply(offset);
    }
  } else if (state_ == kReset) {
    for (auto &anim : animations_) {
      anim.Apply(0.0);
    }
    state_ = kIdle;
  } else if (state_ == kSnapshot) {
    for (auto &anim : animations_) {
      anim.Apply(start_offset_);
    }
    state_ = kPaused;
  }
}

void AnimatedEntity::PreRender() {}
void AnimatedEntity::PostRender() {}
void AnimatedEntity::Destroy() {}

}