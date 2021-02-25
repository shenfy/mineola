#ifndef MINEOLA_MESHIO_H
#define MINEOLA_MESHIO_H

#include <vector>
#include <memory>
#include <optional>
#include <istream>

namespace mineola {

class SceneNode;

namespace mesh_io {

bool LoadPLY(const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask);

bool LoadPLYFromStream(std::istream &ins,
  const char *name,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask);

}} //namespace

#endif
