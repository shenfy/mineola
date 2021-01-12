#ifndef MINEOLA_SCENENODE_H
#define MINEOLA_SCENENODE_H

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <optional>
#include "Rbt.h"
#include "AABB.h"

namespace mineola {
  class Renderable;
  class Light;
  class Camera;

  class SceneNode : public std::enable_shared_from_this<SceneNode> {
  public:
    SceneNode();
    SceneNode(const char *name);
    ~SceneNode();

    const std::weak_ptr<SceneNode> &Parent() const;
    const std::vector<std::shared_ptr<SceneNode>> &Children() const;
    void RemoveChild(const SceneNode &node);
    void RemoveChild(size_t index);
    void RemoveChildren();
    static void LinkTo(std::shared_ptr<SceneNode> child,
      const std::shared_ptr<SceneNode> &parent);

    const glm::vec3 &Position() const;
    void SetPosition(const glm::vec3 &position);
    const glm::quat &Rotation() const;
    void SetRotation(const glm::quat &rotation);
    const glm::vec3 &Scale() const;
    void SetScale(const glm::vec3 &scale);
    const math::Rbt &Rbt() const;
    void SetRbt(const math::Rbt &rbt);

    const std::string &Name() const;
    void SetName(const std::string &name);

    // retrieve cached world tform
    const math::Rbt &WorldRbt() const;
    const glm::vec3 &WorldScale() const;

    // set local rbt w.r.t. parent rbt, given world rbt
    void SetWorldRbt(const math::Rbt &world_rbt);

    std::vector<std::shared_ptr<Renderable>> &Renderables();
    const std::vector<std::shared_ptr<Renderable>> &Renderables() const;

    std::vector<std::shared_ptr<Light>> &Lights();
    const std::vector<std::shared_ptr<Light>> &Lights() const;

    std::vector<std::weak_ptr<Camera>> &Cameras();
    const std::vector<std::weak_ptr<Camera>> &Cameras() const;

    void UpdateSubtreeWorldTforms();

    template<class VisitorT>
    void DFTraverse(const VisitorT &visitor) const;
    template<class VisitorT>
    void DFTraverse(const VisitorT &visitor);

    std::optional<AABB> ComputeAABB() const;

    // bfs search
    template<class UnaryPredicate>
    std::shared_ptr<SceneNode> FindIf(UnaryPredicate p);

    template<class UnaryPredicate>
    std::shared_ptr<SceneNode const> FindIf(UnaryPredicate p) const;

    static std::shared_ptr<SceneNode> FindNodeByName(
      const char *name, SceneNode *node);
    static std::shared_ptr<SceneNode const> FindNodeByName(
      const char *name, const SceneNode *node);

  protected:
    math::Rbt rbt_;
    glm::vec3 scale_;

    std::weak_ptr<SceneNode> parent_;

    std::vector<std::shared_ptr<Renderable>> renderables_;
    std::vector<std::shared_ptr<SceneNode>> children_;
    std::vector<std::shared_ptr<Light>> lights_;
    std::vector<std::weak_ptr<Camera>> cameras_;

    // cache world transform
    math::Rbt world_rbt_;
    glm::vec3 world_scale_;

    std::string name_;
  };

  template<class VisitorT>
  void SceneNode::DFTraverse(const VisitorT &visitor) const {
    visitor(*this);
    for (auto &child : children_)
      child->DFTraverse(visitor);
  }

  template<class VisitorT>
  void SceneNode::DFTraverse(const VisitorT &visitor) {
    visitor(*this);
    for (auto &child : children_)
      child->DFTraverse(visitor);
  }

  template<class UnaryPredicate>
  std::shared_ptr<SceneNode> SceneNode::FindIf(UnaryPredicate p) {
    if (p(*this))
      return shared_from_this();
    for (auto &child : children_) {
      auto ret = child->FindIf(p);
      if (ret) {
        return ret;
      }
    }
    return nullptr;
  }

  template<class UnaryPredicate>
  std::shared_ptr<SceneNode const> SceneNode::FindIf(UnaryPredicate p) const {
    if (p(*this))
      return shared_from_this();
    for (auto &child : children_) {
      auto ret = child->FindIf(p);
      if (ret) {
        return ret;
      }
    }
    return nullptr;
  }
}

#endif //MINEOLA_SCENENODE_H
