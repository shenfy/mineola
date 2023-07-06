#include "prefix.h"
#include <mineola/MeshIO.h>
#include <mineola/PolygonSoupSerialization.h>
#include <mineola/PolygonSoupLoader.h>
#include <mineola/SceneNode.h>
#include <mineola/Engine.h>
#include <mineola/Renderable.h>

namespace {

using namespace mineola;

bool LoadPolygonSoup(const PolygonSoup &soup,
  const char *name,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask) {

  auto renderable = std::make_shared<Renderable>();
  if (!primitive_helper::BuildFromPolygonSoup(soup, name, *renderable)) {
    return false;
  }

  renderable->SetEffect(std::move(effect));
  if (shadowmap_effect) {
    renderable->SetShadowmapEffect(std::move(*shadowmap_effect));
  }
  renderable->SetLayerMask(layer_mask);

  auto node = std::make_shared<SceneNode>();
  node->Renderables().push_back(renderable);
  SceneNode::LinkTo(node, parent_node);

  return true;
}

}

namespace mineola { namespace mesh_io {

bool LoadPLY(const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask) {

  PolygonSoup soup;
  if (!LoadSoupFromPLY(fn, soup)) {
    return false;
  }

  if (!soup.has_vertex_normal) {
    soup.ComputeVertexNormal();
  }

  return LoadPolygonSoup(soup, fn, parent_node,
    std::move(effect), std::move(shadowmap_effect), layer_mask);
}

bool LoadPLYFromStream(std::istream &ins,
  const char *name,
  const std::shared_ptr<SceneNode> &parent_node,
  std::string effect,
  std::optional<std::string> shadowmap_effect,
  int layer_mask) {

  PolygonSoup soup;
  if (!LoadSoupFromPLY(ins, soup)) {
    return false;
  }

  if (!soup.has_vertex_normal) {
    soup.ComputeVertexNormal();
  }

  return LoadPolygonSoup(soup, name, parent_node,
    std::move(effect), std::move(shadowmap_effect), layer_mask);
}


}} //end namespace
