//
//  TestRenderApp.m
//  mineola
//
//  Created by Fangyang Shen on 2018/9/11.
//  Copyright © 2018 Fangyang Shen. All rights reserved.
//
#import "RenderTest.h"
#include <memory>
#include <string>
#include <mineola/glutility.h>
#include <mineola/Engine.h>
#include <mineola/CameraController.h>
#include <mineola/SceneNode.h>
#include <mineola/SceneLoader.h>
#define MINEOLA_USE_STBIMAGE
#include <mineola/STBImagePlugin.h>
#include <mineola/GLTFLoader.h>
#include <mineola/PrefabHelper.h>
#include <mineola/EnvLight.h>
#include <mineola/AnimatedEntity.h>

namespace {

static const std::string kSceneFilename = "gltf.json";
static const std::string kConfigStr = R"({
  "geometries": [
    {
      "filename": "CesiumMan.glb",
      "effect": "mineola:effect:pbr_srgb",
      "shadowmap": false,
      "node": "geometry",
      "layer": 0
    }
  ]
})";

using namespace mineola;
bool FindEnvLight(const std::shared_ptr<SceneNode> &node) {
  bool result = false;

  node->DFTraverse([&result](auto &node) {
    for (auto &light: node.Lights()) {
      if (bd_cast<EnvLight>(light)) {
        result = true;
      }
    }
  });

  return result;
}

}

@interface RenderTest() {
  std::shared_ptr<mineola::CameraController> _camCtrl;
}

@end

@implementation RenderTest

- (void)initScene {
  using namespace mineola;

  auto &en = Engine::Instance();

  std::string resource_path = [[[NSBundle mainBundle] resourcePath] UTF8String];
  en.ResrcMgr().AddSearchPath(resource_path.c_str());

  en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);

  BuildSceneFromConfigFile(kSceneFilename.c_str(), {});

  bool has_env_light = FindEnvLight(en.Scene());

  BuildSceneFromConfig(kConfigStr.c_str(), {
    std::bind(gltf::LoadScene,
      std::placeholders::_1,
      std::placeholders::_2,
      std::placeholders::_3,
      std::placeholders::_4,
      std::placeholders::_5,
      has_env_light)
  });

  if (has_env_light) {
    prefab_helper::CreateSkybox(RenderPass::RENDER_LAYER_ALL, true, *en.Scene());
  }

  _camCtrl = std::make_shared<ArcballController>();
  _camCtrl->BindToNode("camera");
  _camCtrl->Activate();
  bd_cast<ArcballController>(_camCtrl)->SetSpeed(0.1f);

  en.ChangeCamera("main", false);

  en.EntityMgr().Transform([](const std::string &name, auto &entity) {
    auto anim_entity = bd_cast<AnimatedEntity>(entity);
    if (anim_entity) {
      anim_entity->SetPlayMode(AnimatedEntity::kPlayLoop);
      anim_entity->Play();
    }
  });
}

@end
