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
    std::string,  // effect name
    std::optional<std::string>, // shadowmap effect name
    int  // layer id
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
