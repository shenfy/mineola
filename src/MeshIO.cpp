#include "prefix.h"
#include "../include/MeshIO.h"
#include "../include/PolygonSoupSerialization.h"
#include "../include/PolygonSoupLoader.h"
#include "../include/SceneNode.h"
#include "../include/Engine.h"
#include "../include/Renderable.h"

namespace mineola { namespace mesh_io {

bool LoadPLY(const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  const char *effect_name,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &) {

  PolygonSoup soup;
  if (!LoadSoupFromPLY(fn, soup))
    return false;

  auto renderable = std::make_shared<Renderable>();
  if (!primitive_helper::BuildFromPolygonSoup(soup, fn, effect_name, *renderable))
    return false;

  renderable->SetLayerMask(layer_mask);

  auto node = std::make_shared<SceneNode>();
  node->Renderables().push_back(renderable);
  SceneNode::LinkTo(node, parent_node);

  return true;
}

bool LoadPLYFromStream(std::istream &ins,
  const char *name,
  const std::shared_ptr<SceneNode> &parent_node,
  const char *effect_name,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &) {

  PolygonSoup soup;
  if (!LoadSoupFromPLY(ins, soup))
    return false;

  auto renderable = std::make_shared<Renderable>();
  if (!primitive_helper::BuildFromPolygonSoup(soup, name, effect_name, *renderable))
    return false;

  renderable->SetLayerMask(layer_mask);

  auto node = std::make_shared<SceneNode>();
  node->Renderables().push_back(renderable);
  SceneNode::LinkTo(node, parent_node);

  return true;
}


}} //end namespace
