#include <mineola/CameraController.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <mineola/Engine.h>
#include <mineola/Camera.h>
#include <mineola/Framebuffer.h>
#include <mineola/SceneNode.h>

namespace mineola {

CameraController::CameraController() {
}

CameraController::~CameraController() {
}

void CameraController::BindToNode(const std::string &name) {
  auto &en = Engine::Instance();
  auto node = SceneNode::FindNodeByName(name.c_str(), en.Scene().get());
  if (node) {
    node_ = node;
  }
}

void CameraController::ApplyToCamera() {
  auto node = node_.lock();
  if (node) {
    node->SetWorldRbt(cam_rbt_);
  }
}

}
