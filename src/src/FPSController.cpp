#include "prefix.h"
#include <boost/bind/bind.hpp>
#include <mineola/CameraController.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mineola/Engine.h>
#include <mineola/Camera.h>
#include <mineola/Viewport.h>
#include <mineola/Framebuffer.h>
#include <mineola/SceneNode.h>
#include <mineola/MathHelper.h>

namespace mineola {

extern glm::vec3 CalcArcballDirection(const glm::vec2 &dir, float radius);
namespace {
  glm::mat4 RollCorrection(const glm::mat4 &mat, const glm::vec3 &up_dir) {
    glm::mat4 result = mat;
    glm::vec3 z = mat[2];
    glm::vec3 x = glm::cross(up_dir, z);
    if (glm::length(x) < 0.001f)
      return mat;
    x = glm::normalize(x);
    result[0] = glm::vec4(x, 0);
    result[1] = glm::vec4(glm::cross(z, x), 0);
    return result;
  }

  math::Rbt RollCorrection(const math::Rbt &rbt, const glm::vec3 &up_dir) {
    glm::vec3 eye = rbt.Translation();
    glm::vec4 target = rbt * glm::vec4(0, 0, -1, 1);
    return math::Rbt::LookAt(eye, glm::vec3(target), up_dir);
  }
}

FPSController::FPSController() {
}

FPSController::~FPSController() {
}

bool FPSController::Activate() {
  auto &en = Engine::Instance();
  auto shared_this = shared_from_this();
  mouse_btn_conn_ = en.AddMouseButtonCallback(
    Engine::mouse_btn_callback_t(&FPSController::OnMouseButton, this, boost::placeholders::_1,
      boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4)
      .track_foreign(shared_this));
  mouse_move_conn_ = en.AddMouseMoveCallback(
    Engine::mouse_move_callback_t(&FPSController::OnMouseMove, this, boost::placeholders::_1,
      boost::placeholders::_2)
      .track_foreign(shared_this));
  keyboard_conn_ = en.AddKeyboardCallback(
    Engine::keyboard_callback_t(&FPSController::OnKeyboard, this, boost::placeholders::_1,
      boost::placeholders::_2)
      .track_foreign(shared_this));
  frame_move_conn_ = en.AddFrameMoveCallback(
    Engine::frame_move_callback_t(&FPSController::OnFrameMove, this, boost::placeholders::_1,
      boost::placeholders::_2)
      .track_foreign(shared_this));
  return true;
}

void FPSController::Deactivate() {
  mouse_btn_conn_.disconnect();
  mouse_move_conn_.disconnect();
  keyboard_conn_.disconnect();
  frame_move_conn_.disconnect();
}

void FPSController::OnMouseButton(uint8_t button, uint8_t action, int x, int y) {
  auto node = node_.lock();
  if (!node)
    return;

  if (button == Engine::MOUSE_RBUTTON) {
    if (action == Engine::BUTTON_DOWN) {
      rbutton_down_ = true;
      const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);
      screen_center_ = glm::vec2(vp->width * 0.5f, vp->height * 0.5f);
      radius_ = std::min(screen_center_.x, screen_center_.y) * 0.8f;
      glm::vec2 cursor((float)x, (float)(vp->height - y));
      start_dir_ = CalcArcballDirection(cursor - screen_center_, radius_);
      start_cam_rbt_ = node->WorldRbt();
    } else if (action == Engine::BUTTON_UP) {
      rbutton_down_ = false;
    }
  }
}

void FPSController::OnMouseMove(int x, int y) {
  if (rbutton_down_) {
    glm::vec2 cursor((float)x, screen_center_.y * 2 - (float)y);
    glm::vec3 cur_dir = CalcArcballDirection(cursor - screen_center_, radius_);
    glm::quat q1(0.0f, -start_dir_.x, -start_dir_.y, -start_dir_.z);
    glm::quat q2(0.0f, cur_dir.x, cur_dir.y, cur_dir.z);
    math::Rbt rotation = math::Rbt::Inv(math::Rbt(q2 * q1));

    math::Rbt cam_rbt = math::Rbt::DoMToOWrtA(rotation, start_cam_rbt_, start_cam_rbt_);
    cam_rbt_ = RollCorrection(cam_rbt, up_dir_);
    ApplyToCamera();
  }
}

void FPSController::OnKeyboard(uint32_t key, uint8_t action) {
  bool down = action == Engine::BUTTON_DOWN;
  if (key == 'W') {
    if (down)
      moving_forward_ = 1;
    else if (moving_forward_ == 1)
      moving_forward_ = 0;
  } else if (key == 'S') {
    if (down)
      moving_forward_ = -1;
    else if (moving_forward_ == -1)
      moving_forward_ = 0;
  } else if (key == 'A') {
    if (down)
      moving_left_ = 1;
    else if (moving_left_ == 1)
      moving_left_ = 0;
  } else if (key == 'D') {
    if (down)
      moving_left_ = -1;
    else if (moving_left_ == -1)
      moving_left_ = 0;
  } else if (key == 'E') {
    if (down)
      moving_up_ = 1;
    else if (moving_up_ == 1)
      moving_up_ = 0;
  } else if (key == 'Q') {
    if (down)
      moving_up_ = -1;
    else if (moving_up_ == -1)
      moving_up_ = 0;
  }
}

void FPSController::OnFrameMove(double now, double frame_time) {
  if (moving_left_ == 0 && moving_forward_ == 0 && moving_up_ == 0)
    return;
  auto node = node_.lock();
  if (!node)
    return;

  glm::vec3 forward = (float)frame_time / 1000 * moving_forward_ * glm::vec3(0, 0, -1) * move_speed_;
  glm::vec3 left = (float)frame_time / 1000 * moving_left_ * glm::vec3(-1, 0, 0) * move_speed_;
  math::Rbt cam_rbt = node->WorldRbt();
  cam_rbt = math::Rbt::DoMToOWrtA(math::Rbt(forward + left), cam_rbt, cam_rbt);
  glm::vec3 up = (float)frame_time / 1000 * moving_up_ * up_dir_ * move_speed_;
  math::Rbt mix = cam_rbt.TranslationRbt();
  cam_rbt_ = math::Rbt::DoMToOWrtA(math::Rbt(up), cam_rbt, mix);
  start_cam_rbt_.Translation() = cam_rbt.Translation();
  ApplyToCamera();
}

void FPSController::SetUpDir(const glm::vec3 &up) {
  up_dir_ = up;
}

void FPSController::SetMoveSpeed(float speed) {
  move_speed_ = speed;
}

}
