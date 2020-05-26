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

const float TurntableController::kMaxDist = 1e16f;

TurntableController::TurntableController()
  : button_down_(false) {
}

TurntableController::~TurntableController() {
}

bool TurntableController::Activate() {
  auto &en = Engine::Instance();
  mouse_btn_conn_ = en.AddMouseButtonCallback(
    Engine::mouse_btn_callback_t(&TurntableController::OnMouseButton, this, _1, _2, _3, _4)
      .track_foreign(shared_from_this()));
  mouse_move_conn_ = en.AddMouseMoveCallback(
    Engine::mouse_move_callback_t(&TurntableController::OnMouseMove, this, _1, _2)
      .track_foreign(shared_from_this()));
  mouse_scroll_conn_ = en.AddMouseScrollCallback(
    Engine::mouse_scroll_callback_t(&TurntableController::OnMouseScroll, this, _1, _2)
      .track_foreign(shared_from_this()));
  pinch_conn_ = en.AddPinchCallback(
    Engine::pinch_callback_t(&TurntableController::OnPinch, this, _1)
      .track_foreign(shared_from_this()));
  return true;
}

void TurntableController::Deactivate() {
  mouse_btn_conn_.disconnect();
  mouse_move_conn_.disconnect();
  mouse_scroll_conn_.disconnect();
  pinch_conn_.disconnect();
}

void TurntableController::OnMouseButton(uint8_t button, uint8_t action, int x, int y) {
  auto node = node_.lock();
  auto target_node = target_node_.lock();
  if (!node || !target_node) {
    return;
  }

  if (button == Engine::MOUSE_LBUTTON) {
    if (action == Engine::BUTTON_DOWN) {
      const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);
      start_cursor_ = glm::vec2((float)x / vp->width, (float)y / vp->height);
      start_cam_rbt_ = node->WorldRbt();
      start_target_rbt_ = target_node->WorldRbt();
      button_down_ = true;
    } else if (action == Engine::BUTTON_UP) {
      button_down_ = false;
    }
  }
}

void TurntableController::OnMouseMove(int x, int y) {
  if (button_down_) {  // lbutton is being held
    const auto &vp = Engine::Instance().GetScrFramebuffer()->GetViewport(0);
    glm::vec2 cursor((float)x / vp->width, (float)y / vp->height);
    glm::vec2 shift = cursor - start_cursor_;

    // rotate around target y
    math::Rbt roty(glm::angleAxis(-shift.x * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)));
    math::Rbt cam_rbt = math::Rbt::DoMToOWrtA(roty, start_cam_rbt_, start_target_rbt_);

    // rotate around camera x
    math::Rbt aux_frame(start_target_rbt_.Translation(), cam_rbt.Rotation());
    math::Rbt rotx(glm::angleAxis(-shift.y * glm::pi<float>() * 0.5f,
      glm::vec3(1.0f, 0.0f, 0.0f)));
    cam_rbt_ = math::Rbt::DoMToOWrtA(rotx, cam_rbt, aux_frame);

    ApplyToCamera();
  }
}

void TurntableController::SetTarget(const std::string &node_name) {
  auto &en = Engine::Instance();
  auto target_node = SceneNode::FindNodeByName(node_name.c_str(), en.Scene().get());
  if (target_node) {
    target_node_ = target_node;
  }
}

void TurntableController::SetSpeed(float speed) {
  speed_ = std::max(speed, 1e-5f);
}

void TurntableController::OnMouseScroll(int x, int y) {
  if (button_down_ || y == 0)  // do nothing if rotating
    return;

  auto node = node_.lock();
  if (!node) {
    return;
  }

  float z_dir = y > 0 ? -1.0f : 1.0f;
  glm::vec3 move_dir(0.0f, 0.0f, z_dir * speed_);

  math::Rbt trans(move_dir);
  math::Rbt cam_rbt = node->WorldRbt();
  cam_rbt_ = math::Rbt::DoMToOWrtA(trans, cam_rbt, cam_rbt);

  ApplyToCamera();
}

void TurntableController::OnPinch(float scale) {
  auto node = node_.lock();
  auto target_node = target_node_.lock();
  if (!node || !target_node) {
    return;
  }

  auto cam_rbt_wc = node->WorldRbt();
  auto old_pos_wc = cam_rbt_wc.Translation();
  auto target_rbt = target_node->WorldRbt();
  auto target_wc = target_rbt.Translation();

  float old_dist = glm::distance(target_wc, old_pos_wc);
  float new_dist = old_dist / scale;
  if (new_dist > kMaxDist) {
    new_dist = kMaxDist;
  }

  glm::vec3 trans_wc = glm::normalize(target_wc - old_pos_wc) * (old_dist - new_dist);
  cam_rbt_ = math::Rbt(trans_wc) * cam_rbt_wc;

  ApplyToCamera();
}

}
