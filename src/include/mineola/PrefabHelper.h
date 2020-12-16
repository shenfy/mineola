#ifndef MINEOLA_PREFABHELPER_H
#define MINEOLA_PREFABHELPER_H

namespace mineola {

class SceneNode;

namespace prefab_helper {

bool CreateSkybox(int layer_mask, bool srgb, SceneNode &node);

void CreateAxes(int layer_mask, SceneNode &node);

}}

#endif
