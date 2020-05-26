#ifndef MINEOLA_MESHIO_H
#define MINEOLA_MESHIO_H

#include <vector>
#include <memory>
#include <istream>

namespace mineola {
  
  class SceneNode;
  
  namespace mesh_io {

    bool LoadPLY(const char *fn,
      const std::shared_ptr<SceneNode> &parent_node,
      const char *effect,
      int layer_mask,
      const std::vector<std::pair<std::string, std::string>> &inject_textures);

    bool LoadPLYFromStream(std::istream &ins,
      const char *name,
      const std::shared_ptr<SceneNode> &parent_node,
      const char *effect,
      int layer_mask,
      const std::vector<std::pair<std::string, std::string>> &inject_textures);

}} //namespace

#endif
