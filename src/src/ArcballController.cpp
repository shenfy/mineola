#include "prefix.h"
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

const float ArcballController::kMaxDist = 1e16f;

ArcballController::ArcballController()
  : lbutton_down_(false),
  mbutton_down_(false) {
}

ArcballController::~ArcballController() {
}

bool ArcballController::Activate() {
  auto &en = Engine::Instance();
  mouse_btn_conn_ = en.AddMouseButtonCallback(
    Engine::mouse_btn_callback_t(&ArcballController::OnMouseButton, this,
      boost::placeholders::_1,
      boost::placeholders::_2,
      boost::placeholders::_3,
      boost::placeholders::_4)
      .track_foreign(shared_from_this()));
  mouse_move_conn_ = en.AddMouseMoveCallback(
    Engine::mouse_move_callback_t(&ArcballController::OnMouseMove, this,
    boost::placeholders::_1,
    boost::placeholders::_2)
      .track_foreign(shared_from_this()));
  mouse_scroll_conn_ = en.AddMouseScrollCallback(
    Engine::mouse_scroll_callback_t(&ArcballController::OnMouseScroll, this,
    boost::placeholders::_1,
    boost::placeholders::_2)
      .track_foreign(shared_from_this()));
  pinch_conn_ = en.AddPinchCallback(
    Engine::pinch_callback_t(&ArcballController::OnPinch, this,
    boost::placeholders::_1)
      .track_foreign(shared_from_this()));
  return true;
}

void ArcballController::Deactivate() {
  mouse_btn_conn_.disconnect();
  mouse_move_conn_.disconnect();
  mouse_scroll_conn_.disconnect();
  pinch_conn_.disconnect();
}

glm::vec3 CalcArcballDirection(const glm::vec2 &dir, float radius) {
  float sqr_length = glm::dot(dir, dir);
  float sqr_radius = radius * radius;
  if (sqr_length >= sqr_radius)
    return glm::normalize(glm::vec3(dir, 0));
  else
    return glm::normalize(glm::vec3(dir, sqrtf(sqr_radius - sqr_length)));
}

void ArcballController::OnMouseButton(uint8_t button, uint8_t action, int x, int y) {
  auto node = node_.lock();
  if (!node)
    return;
  if (button == Engine::MOUSE_MBUTTON) {
    if (action == Engine::BUTTON_DOWN) {
      // record start dist and position
      start_cam_rbt_ = node->WorldRbt();
      glm::mat4 view_mat = math::Rbt::Inv(start_cam_rbt_).ToMatrix();
      start_target_ec_ = view_mat * glm::vec4(target_, 1.0f);
      const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);
      start_cursor_ = glm::vec2(x, y) / glm::vec2(vp->width, vp->height);
      mbutton_down_ = true;
    } else if (action == Engine::BUTTON_UP) {
      mbutton_down_ = false;
    }
  } else if (button == Engine::MOUSE_LBUTTON) {
    if (action == Engine::BUTTON_DOWN) {
      // record start dir
      const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);
      screen_center_ = glm::vec2(vp->width * 0.5f, vp->height * 0.5f);
      radius_ = std::min(screen_center_.x, screen_center_.y) * 0.99f;
      glm::vec2 cursor((float)x, (float)(vp->height - y));
      start_dir_ = CalcArcballDirection(cursor - screen_center_, radius_);
      start_cam_rbt_ = node->WorldRbt();
      lbutton_down_ = true;
    } else if (action == Engine::BUTTON_UP) {
      lbutton_down_ = false;
    }
  }
}

void ArcballController::OnMouseMove(int x, int y) {
  if (mbutton_down_) {
    const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);

    glm::vec2 shift_sc = glm::vec2(x, y) / glm::vec2(vp->width, vp->height) - start_cursor_;
    glm::vec3 shift_wc = glm::vec3(shift_sc.x, -shift_sc.y, 0.0) * start_target_ec_.z;
    math::Rbt translation(shift_wc);
    cam_rbt_ = math::Rbt::DoMToOWrtA(translation, start_cam_rbt_, start_cam_rbt_);
    ApplyToCamera();

  } else if (lbutton_down_) {  // lbutton is being held
    glm::vec2 cursor((float)x, screen_center_.y * 2 - (float)y);
    glm::vec3 cur_dir = CalcArcballDirection(cursor - screen_center_, radius_);
    glm::quat q1(0.0f, -start_dir_.x, -start_dir_.y, -start_dir_.z);
    glm::quat q2(0.0f, cur_dir.x, cur_dir.y, cur_dir.z);
    math::Rbt rotation = math::Rbt::Inv(math::Rbt(q2 * q1));

    math::Rbt mix(target_, start_cam_rbt_.Rotation());
    cam_rbt_ = math::Rbt::DoMToOWrtA(rotation, start_cam_rbt_, mix);
    ApplyToCamera();
  }
}

void ArcballController::SetSpeed(float speed) {
  speed_ = std::max(speed, 1e-5f);
}

void ArcballController::SetTarget(const glm::vec3 &target) {
  target_ = target;
}

void ArcballController::OnMouseScroll(int x, int y) {
  if (lbutton_down_ || y == 0)  // do nothing if rotating
    return;

  auto node = node_.lock();
  if (!node)
    return;

  float z_dir = y > 0 ? -1.0f : 1.0f;
  glm::vec3 move_dir(0.0f, 0.0f, z_dir * speed_);

  math::Rbt trans(move_dir);
  math::Rbt cam_rbt = node->WorldRbt();
  cam_rbt_ = math::Rbt::DoMToOWrtA(trans, cam_rbt, cam_rbt);

  ApplyToCamera();
}

void ArcballController::OnPinch(float scale) {
  auto node = node_.lock();
  if (!node)
    return;

  auto cam_rbt_wc = node->WorldRbt();
  auto old_pos_wc = cam_rbt_wc.Translation();

  float old_dist = glm::distance(target_, old_pos_wc);
  float new_dist = old_dist / scale;
  if (new_dist > kMaxDist) {
    new_dist = kMaxDist;
  }

  glm::vec3 trans_wc = glm::normalize(target_ - old_pos_wc) * (old_dist - new_dist);
  math::Rbt trans_rbt(trans_wc);

  cam_rbt_ = trans_rbt * cam_rbt_wc;

  ApplyToCamera();
}

}
