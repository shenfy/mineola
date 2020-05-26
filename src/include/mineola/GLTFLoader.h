#ifndef MINEOLA_GLTFLOADER_H
#define MINEOLA_GLTFLOADER_H

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
  const char *effect,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &inject_textures);

}} //end namespace

#endif
