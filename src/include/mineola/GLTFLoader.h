#ifndef MINEOLA_GLTFLOADER_H
#define MINEOLA_GLTFLOADER_H

#include <optional>
#include "VertexType.h"

namespace imgpp {
  class Img;
}

namespace mineola {
class SceneNode;

namespace gltf {

bool LoadScene(
  const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask,
  bool use_env_light);

}} //end namespace

#endif
