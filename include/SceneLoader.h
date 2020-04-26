#ifndef MINEOLA_SCENELOADER_H
#define MINEOLA_SCENELOADER_H

#include <vector>
#include <functional>
#include <memory>
#include <istream>

namespace mineola {

class SceneNode;
using GeometryLoaderT = std::function<
  bool (
    const char *,  // path
    const std::shared_ptr<SceneNode> &,  // dst SceneNode
    const char *,  // effect name
    int,  // layer id
    const std::vector<std::pair<std::string, std::string>> &  // inject textures
  )
>;
using GeometryLoaderVecT = std::vector<GeometryLoaderT>;

bool BuildSceneFromConfig(const char *config_str,
  const GeometryLoaderVecT &geometry_loaders = GeometryLoaderVecT());

bool BuildSceneFromConfigStream(std::istream &ins,
  const GeometryLoaderVecT &geometry_loaders = GeometryLoaderVecT());

bool BuildSceneFromConfigFile(const char *filename,
  const GeometryLoaderVecT &geometry_loaders = GeometryLoaderVecT());

} //namespace

#endif