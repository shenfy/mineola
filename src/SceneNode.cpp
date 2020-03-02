#include "prefix.h"
#include "../include/Light.h"
#include "../include/Camera.h"
#include "../include/SceneNode.h"
#include "../include/Renderable.h"

namespace mineola {
  SceneNode::SceneNode() :
    scale_(1, 1, 1), world_scale_(1, 1, 1) {
  }

  SceneNode::SceneNode(const char *name) :
    scale_(1, 1, 1), world_scale_(1, 1, 1),
    name_(name) {
  }

  SceneNode::~SceneNode() {
  }

  const std::weak_ptr<SceneNode>& SceneNode::Parent() const {
    return parent_;
  }

  const std::vector<std::shared_ptr<SceneNode>>& SceneNode::Children() const {
    return children_;
  }

  void SceneNode::RemoveChild(const SceneNode &node) {
    for (size_t i = 0; i < children_.size(); ++i) {
      if (children_[i].get() == &node) {
        RemoveChild(i);
        return;
      }
    }
  }

  void SceneNode::RemoveChild(size_t index) {
    if (index >= children_.size())
      return;
    children_[index]->parent_.reset();
    children_.erase(children_.begin() + index);
  }

  void SceneNode::RemoveChildren() {
    for (size_t i = 0; i < children_.size(); ++i)
      children_[i]->parent_.reset();
    children_.clear();
  }

  void SceneNode::LinkTo(const std::shared_ptr<SceneNode> &child, const std::shared_ptr<SceneNode> &parent) {
    if (!child)
      return;
    auto old_parent = child->parent_.lock();
    if (old_parent)
      old_parent->RemoveChild(*child);
    if (parent)
      parent->children_.push_back(child);
    child->parent_ = parent;
  }

  const glm::vec3& SceneNode::Position() const {
    return rbt_.Translation();
  }

  void SceneNode::SetPosition(const glm::vec3 &position) {
    rbt_.Translation() = position;
  }

  const glm::quat& SceneNode::Rotation() const {
    return rbt_.Rotation();
  }

  void SceneNode::SetRotation(const glm::quat &rotation) {
    rbt_.Rotation() = rotation;
  }

  const glm::vec3& SceneNode::Scale() const {
    return scale_;
  }

  void SceneNode::SetScale(const glm::vec3 &scale) {
    scale_ = scale;
  }

  const math::Rbt& SceneNode::Rbt() const {
    return rbt_;
  }

  void SceneNode::SetRbt(const math::Rbt &rbt) {
    rbt_ = rbt;
  }

  const std::string &SceneNode::Name() const {
    return name_;
  }

  void SceneNode::SetName(const std::string &name) {
    name_ = name;
  }

  const math::Rbt& SceneNode::WorldRbt() const {
    return world_rbt_;
  }

  const glm::vec3 &SceneNode::WorldScale() const {
    return world_scale_;
  }

  void SceneNode::SetWorldRbt(const math::Rbt &rbt) {
    auto parent = parent_.lock();
    rbt_ = math::Rbt::Inv(parent->world_rbt_) * rbt;
    rbt_.Translation() /= parent->world_scale_;
    world_rbt_ = rbt;
  }


  std::vector<std::shared_ptr<Renderable>>& SceneNode::Renderables() {
    return renderables_;
  }

  const std::vector<std::shared_ptr<Renderable>>& SceneNode::Renderables() const {
    return renderables_;
  }

  std::vector<std::shared_ptr<Light>>& SceneNode::Lights() {
    return lights_;
  }

  const std::vector<std::shared_ptr<Light>>& SceneNode::Lights() const {
    return lights_;
  }

  std::vector<std::weak_ptr<Camera>>& SceneNode::Cameras() {
    return cameras_;
  }

  const std::vector<std::weak_ptr<Camera>>& SceneNode::Cameras() const {
    return cameras_;
  }

  void SceneNode::UpdateSubtreeWorldTforms() {
    DFTraverse([](SceneNode &node) {
      auto parent = node.parent_.lock();
      if (parent) {
        math::Rbt scaled_local = node.rbt_;
        scaled_local.Translation() = parent->world_scale_ * scaled_local.Translation();
        node.world_rbt_ = parent->world_rbt_ * scaled_local;
        node.world_scale_ = parent->world_scale_ * node.scale_;
      } else {
        node.world_rbt_ = node.rbt_;
        node.world_scale_ = node.scale_;
      }

      // update light transforms
      for (auto &light : node.lights_) {
        light->UpdateLightTransform(node.world_rbt_);
      }

      // update camera transforms
      for (auto &weak_camera : node.cameras_) {
        auto camera = weak_camera.lock();
        if (camera) {
          camera->SetViewMatrix(glm::inverse(node.world_rbt_.ToMatrix()));
        }
      }
    });
  }

  std::shared_ptr<SceneNode> SceneNode::FindNodeByName(
    const char *name,
    const std::shared_ptr<SceneNode> &start_node) {

    std::string name_str = name;
    return start_node->FindIf(
      [&name_str](const SceneNode &node) {
        return node.Name() == name_str;
      });
  }
}
