#ifndef MINEOLA_GLTFLOADER_H
#define MINEOLA_GLTFLOADER_H

#include "VertexType.h"

namespace imgpp {
  class Img;
}

namespace mineola {
class SceneNode;

namespace texture_helper {
using texture_loader_t = std::add_pointer<bool(const char*, imgpp::Img &img)>::type;
using texture_mem_loader_t = std::add_pointer<bool(const char*, uint32_t, imgpp::Img &img)>::type;
}

namespace gltf {

bool LoadScene(
  texture_helper::texture_loader_t texture_file_loader,
  texture_helper::texture_mem_loader_t texture_mem_loader,
  const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  const char *effect,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &inject_textures);

}} //end namespace

#endif
