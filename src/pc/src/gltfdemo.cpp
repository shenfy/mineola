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

std::ostream &operator<<(std::ostream &os, const glm::vec3 &vec) {
  os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
  return os;
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

    if (!gltf_filename_.empty()) {
      en.ResrcMgr().AddSearchPath("resrc");
      BuildSceneFromConfigFile(kSceneFilename.c_str(), {});

      // Load gltf
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

      // Adjust camera, light and shadow
      auto geometry_node = SceneNode::FindNodeByName("geometry", en.Scene().get());
      std::optional<AABB> aabb;
      if (geometry_node) {
        aabb = geometry_node->ComputeAABB();
      }

      if (aabb) {
        auto center = aabb->Center();
        auto extent = aabb->Extent();
        auto max_extent = std::max(std::max(extent.x, extent.y), extent.z);
        std::cout << "Model center: " << center << ", extent: " << extent << std::endl;

        // camera
        auto camera_node = SceneNode::FindNodeByName("camera", en.Scene().get());
        if (camera_node) {
          auto pos = center;
          pos.z += 2 * max_extent;
          camera_node->SetPosition(pos);
          for (auto &camera: camera_node->Cameras()) {
            auto c = camera.lock();
            if (c) {
              c->SetProjParams(c->Fov(), max_extent / 10.0f, max_extent * 10.0f);
            }
          }
        }

        // camera control
        auto cam_ctrl = std::make_shared<ArcballController>();
        cam_ctrl->SetTarget(center);
        cam_ctrl->SetSpeed(max_extent / 10.0f);
        cam_ctrl->BindToNode("camera");
        cam_ctrl->Activate();
        cam_ctrl_ = cam_ctrl;

        // light
        auto light_node = SceneNode::FindNodeByName("dirlight", en.Scene().get());
        if (light_node) {
          auto pos = center;
          pos.y += extent.y;
          light_node->SetPosition(pos);

          for (auto &light: light_node->Lights()) {
            auto dir_light = bd_cast<PointDirLight>(light);
            if (dir_light) {
              // Because light node up is (1, 0, 0), so left/right is world z,
              // bottom/top is world x, forward/backward is world y
              dir_light->SetOrthoProjParams(
                -extent.z * 0.6f, extent.z * 0.6f,
                -extent.x * 0.6f, extent.x * 0.6f,
                extent.y * 0.25f, extent.y * 2.0f
              );
            }
          }
        }

        // shadow
        auto shadow_node = SceneNode::FindNodeByName("shadow", en.Scene().get());
        if (shadow_node) {
          auto pos = center;
          pos.y -= extent.y * 0.5f;
          shadow_node->SetPosition(pos);
          shadow_node->SetScale(extent * 0.6f);
        }
      }
    }

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
