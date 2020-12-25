#include <jni.h>

#include <mineola/Engine.h>
#include <mineola/MeshIO.h>
#include <mineola/SceneLoader.h>
#include <mineola/CameraController.h>
#include <mineola/GLTFLoader.h>
#include <mineola/AnimatedEntity.h>
#define MINEOLA_USE_STBIMAGE
#include <mineola/STBImagePlugin.h>

namespace {

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

}

extern "C" {
  JNIEXPORT void JNICALL Java_com_shenfy_mineola_gltfviewer_GLTFViewerLib_initScene(
    JNIEnv *env, jobject obj);
}

JNIEXPORT void JNICALL Java_com_shenfy_mineola_gltfviewer_GLTFViewerLib_initScene(
  JNIEnv *env, jobject obj) {

  using namespace mineola;

  auto &en = Engine::Instance();
  en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);

  BuildSceneFromConfigFile("gltf.json", {gltf::LoadScene});
  BuildSceneFromConfig(kConfigStr.c_str(), {gltf::LoadScene});

  static auto cam_ctrl = std::make_shared<ArcballController>();
  cam_ctrl->BindToNode("camera");
  cam_ctrl->Activate();
  bd_cast<ArcballController>(cam_ctrl)->SetSpeed(0.1f);

  en.ChangeCamera("main", false);

  en.EntityMgr().Transform([](const std::string &name, auto &entity) {
    auto anim_entity = bd_cast<AnimatedEntity>(entity);
    if (anim_entity) {
      anim_entity->SetPlayMode(AnimatedEntity::kPlayLoop);
      anim_entity->Play();
    }
  });

  return;
}
