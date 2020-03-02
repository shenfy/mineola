#ifndef MINEOLA_ANIMATION_H
#define MINEOLA_ANIMATION_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mineola {

class SceneNode;

namespace animation {

struct KeyFrame {
  glm::vec3 translation;
  glm::quat rotation;
  glm::vec3 scale;

  static KeyFrame Lerp(const KeyFrame &f0, const KeyFrame &f1, float t);
  static KeyFrame CubicSpline(
    const KeyFrame &f0, const KeyFrame &t0_out,
    const KeyFrame &f1, const KeyFrame &t1_in, float tan_scale,
    float t);
};

struct Channel {
  enum {kInterpStep, kInterpLinear, kInterpCubicSpline};
  enum {kAnimUnknown = 0, kAnimTranslation = 1, kAnimRotation = 2, kAnimScale = 4};
  enum {kDefaultFPS = 25};

  int type {kAnimUnknown};
  int interp {kInterpLinear};
  int fps {kDefaultFPS};
  std::weak_ptr<SceneNode> target;
  std::vector<KeyFrame> key_frames;

  /**
   * @brief Get length of this channel in ms
   * @details Calculate length from fps and number of key_frames
   * @return Animation length in milliseconds
   */
  double Length() const;

  /**
   * @brief Apply animation to targets
   * @details Interpolate between key frames given offset time, and apply to SceneNodes
   *
   * @param time - relative time (offset) from animation start
   */
  void Apply(double time);
};

class Animation {
public:
  Animation();
  virtual ~Animation();

  void AddChannel(Channel &&channel);
  double Length() const;

  /**
   * @brief Sample channels and apply to targets
   * @details Given the offset time from animation start, call all channels' apply method
   *
   * @param time - relative time (offset) from animation start
   */
  void Apply(double time);

protected:
  std::vector<Channel> channels_;
  double length_{0.0};

};

}}  //namespace


#endif