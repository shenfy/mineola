#include "../include/AppFrame.h"
#include <memory>
#include <iostream>
#include <mineola/AppHelper.h>
#include <mineola/CameraController.h>
#include <mineola/SceneNode.h>
#include <mineola/SceneLoader.h>
#include <mineola/MeshIO.h>
#include <mineola/FileSystem.h>
#define MINEOLA_USE_STBIMAGE
#include <mineola/STBImagePlugin.h>
#include <mineola/GLTFLoader.h>
#include <mineola/PrefabHelper.h>
#include <mineola/TextureHelper.h>
#include <mineola/EnvLight.h>
#include <mineola/AnimatedEntity.h>

namespace {

static const std::string kSceneFilename = "resrc/gltf.json";
static const std::string kConfigPrefix = R"({
  "geometries": [
    {
      "filename": ")";

static const std::string kConfigSuffix = R"(",
      "effect": "mineola:effect:pbr_srgb",
      "shadowmap_effect": "mineola:effect:pbr_srgb",
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

namespace mineola_pc {
using namespace mineola;

void OnKey(uint32_t key, uint32_t action) {
  if (action == Engine::BUTTON_UP) {
    auto &en = Engine::Instance();
    if (key == 'Q') {
      en.SignalTermination();
    } else if (key == 'P') {
      en.EntityMgr().Transform([](const std::string &name, auto &entity) {
        auto anim_entity = bd_cast<AnimatedEntity>(entity);
        if (anim_entity) {
          anim_entity->Play();
        }
      });

    } else if (key == 'S') {
      static double offset = 0.0;
      en.EntityMgr().Transform([](const std::string &name, auto &entity) {
        auto anim_entity = bd_cast<AnimatedEntity>(entity);
        if (anim_entity) {
          anim_entity->Snapshot(offset);
        }
      });
      offset += 10.0;

    } else if (key == 'R') {
    }
  }
}

class GLTFViewerApp : public mineola::AppFrame,
  public std::enable_shared_from_this<GLTFViewerApp> {
public:
  GLTFViewerApp() {}
  virtual ~GLTFViewerApp() {}

  virtual bool InitScene() override {
    AppFrame::InitScene();

    Engine &en = GetEngine();

    en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);
    if (!texture_helper::CreateShadowmapRenderTarget(1024, 1024)) {
      std::cout << "Failed to create shadowmap render target" << std::endl;
    }
    en.RenderPasses().push_back(CreateShadowmapPass());

    if (!gltf_filename_.empty()) {
      en.ResrcMgr().AddSearchPath("resrc");
      BuildSceneFromConfigFile(kSceneFilename.c_str(), {});

      bool has_env_light = FindEnvLight(en.Scene());

      auto cfg = kConfigPrefix + gltf_filename_ + kConfigSuffix;

      BuildSceneFromConfig(cfg.c_str(), {
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
    }

    cam_ctrl_.reset(new ArcballController);
    cam_ctrl_->BindToNode("camera");
    cam_ctrl_->Activate();
    bd_cast<ArcballController>(cam_ctrl_)->SetSpeed(0.1f);

    en.AddKeyboardCallback(OnKey);

    en.ChangeCamera("main", false);

    return true;
  }

  void SetFilename(const char *filename) {
    gltf_filename_ = filename;
  }

private:
  std::string gltf_filename_;
  std::shared_ptr<mineola::CameraController> cam_ctrl_;
};

}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: gltfdemo (gltf filename)" << std::endl;
    return 0;
  }

  std::shared_ptr<mineola_pc::GLTFViewerApp> app(new mineola_pc::GLTFViewerApp());
  app->SetFilename(argv[1]);
  std::cout << "Press P to play animation" << std::endl;
  app->Run(900, 900);
  return 0;
}
