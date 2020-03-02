#include <jni.h>

#include <Engine.h>
#include <MeshIO.h>
#include <SceneLoader.h>
#include <CameraController.h>
#include <GLTFLoader.h>
#define MINEOLA_USE_STBIMAGE
#include <STBImagePlugin.h>

extern "C" {
  JNIEXPORT void JNICALL Java_com_oppentech_androidexample_RenderTestLib_initScene(
    JNIEnv *env, jobject obj);
}

JNIEXPORT void JNICALL Java_com_oppentech_androidexample_RenderTestLib_initScene(
  JNIEnv *env, jobject obj) {

  using namespace mineola;

  auto &en = Engine::Instance();
  {
    namespace ph = std::placeholders;
    auto gltf_loader = std::bind(gltf::LoadScene, STBLoadImageFromFile, STBLoadImageFromMem,
        ph::_1, ph::_2, ph::_3, ph::_4, ph::_5);
    BuildSceneFromConfigFile("gltfscene.json", {gltf_loader});
  }

  en.RenderPasses().push_back(RenderPass());
  en.ChangeCamera("main", false);

  static auto cam_ctrl = std::make_shared<ArcballController>();
  cam_ctrl->BindToNode("camera");
  cam_ctrl->Activate();
  bd_cast<ArcballController>(cam_ctrl)->SetSpeed(0.02f);

  return;
}
