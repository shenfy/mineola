#ifndef MINEOLA_SKIN_H
#define MINEOLA_SKIN_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace mineola {

class SceneNode;

class Skin {
public:
  Skin();
  ~Skin();

  void PreRender(double frame_time, uint32_t pass);

  void SetRootNode(std::shared_ptr<SceneNode> &root_node);
  void SetJointNodes(std::vector<std::weak_ptr<SceneNode>> &&nodes,
    std::vector<glm::mat4> &&inv_bind_mats);

protected:
  std::weak_ptr<SceneNode> root_node_;
  std::vector<std::weak_ptr<SceneNode>> joint_nodes_;
  std::vector<glm::mat4> inv_bind_mats_;

  std::vector<glm::mat4> joint_mats_;
  glm::mat4 root_mat_;

  void CalculateMatrices();
};

} //namespaces

#endif