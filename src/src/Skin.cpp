#include "prefix.h"
#include <mineola/Skin.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mineola/Engine.h>
#include <mineola/SceneNode.h>
#include <mineola/GLEffect.h>

namespace mineola {

Skin::Skin() = default;

Skin::~Skin() = default;

void Skin::PreRender(double frame_time, uint32_t pass) {
  if (joint_nodes_.size() == 0) {
    return;
  }

  CalculateMatrices();

  // send uniforms to GPU
  auto &en = Engine::Instance();
  auto &effect = en.CurrentEffect();
  effect->UploadVariable("_joint_mats[0]", glm::value_ptr(joint_mats_[0]));
}

void Skin::SetRootNode(std::shared_ptr<SceneNode> &node) {
  root_node_ = node;
}

void Skin::SetJointNodes(
  std::vector<std::weak_ptr<SceneNode>> &&nodes,
  std::vector<glm::mat4> &&inv_bind_mats) {

  joint_nodes_ = std::move(nodes);
  size_t num_joints = joint_nodes_.size();
  size_t num_joints_32 = (num_joints + 31) >> 5 << 5;

  inv_bind_mats_ = std::move(inv_bind_mats);
  if (inv_bind_mats_.size() != num_joints) {
    throw std::logic_error("Wrong number of inverse bind matrices for joints!");
  }

  joint_mats_.resize(num_joints_32);  // only mats sent to GPU
}

// collect joint node global transforms and recalculate joint matrices
void Skin::CalculateMatrices() {
  for (size_t idx = 0; idx < joint_nodes_.size(); ++idx) {
    auto node = joint_nodes_[idx].lock();
    auto global_joint_mat = node->WorldRbt().ToMatrix();
    global_joint_mat = glm::scale(global_joint_mat, node->WorldScale());

    joint_mats_[idx] = global_joint_mat * inv_bind_mats_[idx];
  }
}

}
