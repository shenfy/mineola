#ifndef MINEOLA_CAMERACONTROLLER_H
#define MINEOLA_CAMERACONTROLLER_H

#include <memory>
#include <boost/signals2.hpp>
#include <glm/glm.hpp>
#include "Rbt.h"

namespace mineola {

class Camera;
class SceneNode;

class CameraController {
public:
  CameraController();
  virtual ~CameraController();
  virtual bool Activate() {return false;}
  virtual void Deactivate() {}
  virtual void BindToNode(const std::string &name);
  void ApplyToCamera();

protected:
  std::weak_ptr<SceneNode> node_;
  math::Rbt cam_rbt_;
};

class ArcballController :
  public CameraController,
  public std::enable_shared_from_this<ArcballController> {
public:
  ArcballController();
  virtual ~ArcballController();

  virtual bool Activate();
  virtual void Deactivate();

  void OnMouseButton(uint8_t button, uint8_t action, int x, int y);
  void OnMouseMove(int x, int y);
  void OnMouseScroll(int x, int y);
  void OnPinch(float scale);
  void SetSpeed(float speed);
  void SetTarget(const glm::vec3 &target);

protected:
  glm::vec2 screen_center_;
  bool lbutton_down_;
  math::Rbt start_cam_rbt_;
  glm::vec3 start_dir_;
  float radius_;
  float speed_ {0.1f};
  glm::vec3 target_;

  bool mbutton_down_;
  glm::vec4 start_target_ec_;
  glm::vec2 start_cursor_;

  static const float kMaxDist;

  boost::signals2::connection mouse_btn_conn_;
  boost::signals2::connection mouse_move_conn_;
  boost::signals2::connection mouse_scroll_conn_;
  boost::signals2::connection pinch_conn_;
};

class TurntableController :
  public CameraController,
  public std::enable_shared_from_this<TurntableController> {
public:
  TurntableController();
  virtual ~TurntableController();

  virtual bool Activate();
  virtual void Deactivate();

  void OnMouseButton(uint8_t button, uint8_t action, int x, int y);
  void OnMouseMove(int x, int y);
  void OnMouseScroll(int x, int y);
  void OnPinch(float scale);

  void SetTarget(const std::string &node);
  void SetSpeed(float speed);

protected:
  std::weak_ptr<SceneNode> target_node_;
  float speed_ {0.1f};

  math::Rbt start_cam_rbt_;
  math::Rbt start_target_rbt_;
  glm::vec2 start_cursor_;
  bool button_down_;
  static const float kMaxDist;

  boost::signals2::connection mouse_btn_conn_;
  boost::signals2::connection mouse_move_conn_;
  boost::signals2::connection mouse_scroll_conn_;
  boost::signals2::connection pinch_conn_;
};

class FPSController :
  public CameraController,
  public std::enable_shared_from_this<FPSController> {
public:
  FPSController();
  virtual ~FPSController();

  virtual bool Activate() override;
  virtual void Deactivate() override;

  void OnMouseButton(uint8_t button, uint8_t action, int x, int y);
  void OnMouseMove(int x, int y);
  void OnKeyboard(uint32_t key, uint8_t action);
  void OnFrameMove(double now, double frame_time);

  void SetMoveSpeed(float speed);
  void SetUpDir(const glm::vec3 &up);

protected:
  glm::vec3 up_dir_{0, 1, 0};
  bool rbutton_down_{false};
  glm::vec2 screen_center_;
  glm::vec3 start_dir_;
  float radius_;
  math::Rbt start_cam_rbt_;
  int moving_forward_{0};
  int moving_left_{0};
  int moving_up_{0};
  float move_speed_{2.0f};

  boost::signals2::connection mouse_btn_conn_;
  boost::signals2::connection mouse_move_conn_;
  boost::signals2::connection keyboard_conn_;
  boost::signals2::connection frame_move_conn_;
};

}

#endif
