#include "prefix.h"
#include <mineola/Animation.h>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mineola/glutility.h>
#include <mineola/SceneNode.h>

namespace {

  template<typename T>
  T Interpolate(const T &v0, const T &v1, float t, int method) {
    if (method == mineola::animation::Channel::kInterpStep) {
      return v0;
    } else if (method == mineola::animation::Channel::kInterpLinear) {
      return glm::mix(v0, v1, t);
    } else if (method == mineola::animation::Channel::kInterpCubicSpline) {
      return glm::mix(v0, v1, t);
    } else {
      throw std::invalid_argument("Unknown interpolation type!");
    }
  }

  template<>
  glm::quat Interpolate(const glm::quat &q0, const glm::quat &q1, float t, int method) {
    if (method == mineola::animation::Channel::kInterpStep) {
      return q0;
    } else if (method == mineola::animation::Channel::kInterpLinear) {
      return glm::slerp(q0, q1, t);
    } else if (method == mineola::animation::Channel::kInterpCubicSpline) {
      return glm::slerp(q0, q1, t);
    } else {
      throw std::invalid_argument("Unknown interpolation type!");
    }
  }

}

namespace mineola { namespace animation {

KeyFrame KeyFrame::Lerp(const KeyFrame &f0, const KeyFrame &f1, float t) {
  KeyFrame result;
  result.translation = std::move(
    Interpolate(f0.translation, f1.translation, t, Channel::kInterpLinear));
  result.rotation = std::move(
    Interpolate(f0.rotation, f1.rotation, t, Channel::kInterpLinear));
  result.scale = std::move(
    Interpolate(f0.scale, f1.scale, t, Channel::kInterpLinear));
  return result;
}

KeyFrame KeyFrame::CubicSpline(
  const KeyFrame &f0, const KeyFrame &t0_out,
  const KeyFrame &f1, const KeyFrame &t1_in, float tan_scale,
  float t) {

  float t2 = t * t;
  float t3 = t2 * t;
  float c1 = 2.f * t3 - 3.f * t2 + 1.f;
  float c2 = t3 - 2.f * t2 + t;
  float c3 = -2.f * t3 + 3.f * t2;
  float c4 = t3 - t2;

  KeyFrame result;
  result.translation = f0.translation * c1 + t0_out.translation * c2 * tan_scale
    + f1.translation * c3 + t1_in.translation * c4 * tan_scale;
  result.rotation = f0.rotation * c1 + t0_out.rotation * c2 * tan_scale
    + f1.rotation * c3 + t1_in.rotation * c4 * tan_scale;
  result.rotation = glm::normalize(result.rotation);
  result.scale = f0.scale * c1 + t0_out.scale * c2 * tan_scale
    + f1.scale * c3 + t1_in.scale * c4 * tan_scale;

  return result;
}

double Channel::Length() const {
  return (double)(key_frames.size() - 1) / fps * 1000.;
}

void Channel::Apply(double time) {
  double length = Length();
  if (time > length) {
    return;
  }

  auto node = target.lock();
  if (!node) {  // not bound to a SceneNode
    return;
  }

  double frame = time * fps / 1000.0;
  int idx0 = (int)floor(frame);
  int idx1 = idx0 + 1;
  float t = frame - idx0;

  if (idx0 >= 0 && idx0 < (int)key_frames.size() - 1) {  // frame in valid range
    if (type & kAnimTranslation) {
      auto translation = Interpolate(
        key_frames[idx0].translation, key_frames[idx1].translation, t, interp);
      node->SetPosition(translation);
    }

    if (type & kAnimRotation) {
      auto rotation = Interpolate(
        key_frames[idx0].rotation, key_frames[idx1].rotation, t, interp);
      node->SetRotation(rotation);
    }

    if (type & kAnimScale) {
      auto scale = Interpolate(
        key_frames[idx0].scale, key_frames[idx1].scale, t, interp);
      node->SetScale(scale);
    }
  } else {  // before start or after end
    KeyFrame current_frame;
    if (idx0 < 0) {
      current_frame = key_frames.front();
    } else {
      current_frame = key_frames.back();
    }

    if (type & kAnimTranslation) {
      node->SetPosition(current_frame.translation);
    }
    if (type & kAnimRotation) {
      node->SetRotation(current_frame.rotation);
    }
    if (type & kAnimScale) {
      node->SetScale(current_frame.scale);
    }
  }

}

//////////////////////////////////////////////////////////////

Animation::Animation() {
}

Animation::~Animation() {
}

void Animation::AddChannel(Channel &&channel) {
  double new_length = channel.Length();
  if (new_length > length_) {
    length_ = new_length;
  }
  channels_.push_back(std::move(channel));
}

double Animation::Length() const {
  return length_;
}

void Animation::Apply(double time) {
  for (auto &channel : channels_) {
    channel.Apply(time);
  }
}


}}  // namespace
