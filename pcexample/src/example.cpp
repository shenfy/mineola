#include "../include/AppFrame.h"
#include <memory>
#include <iostream>
#include "../../include/AppHelper.h"
#include "../../include/CameraController.h"
#include "../../include/SceneLoader.h"
#include "../../include/MeshIO.h"
#include "../../include/FileSystem.h"

namespace mineola_pc {
using namespace mineola;

class TestApp : public mineola::AppFrame,
  public std::enable_shared_from_this<TestApp> {
public:
  TestApp() {}
  virtual ~TestApp() {}

  virtual bool InitScene() override {
    AppFrame::InitScene();

    Engine &en = GetEngine();

    if (!init_scene_fn_.empty()) {
      BuildSceneFromConfigFile(init_scene_fn_.c_str(), {mesh_io::LoadPLY});
    }

    cam_ctrl_.reset(new TurntableController);
    // cam_ctrl_.reset(new ArcballController);
    cam_ctrl_->BindToNode("camera");
    cam_ctrl_->Activate();
    bd_cast<TurntableController>(cam_ctrl_)->SetSpeed(0.1f);
    bd_cast<TurntableController>(cam_ctrl_)->SetTarget("geometry");

    en.RenderStateMgr().SetClearColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), true);
    en.ChangeCamera("main", false);

    return true;
  }

  void SetScenePath(const char *filename) {
    init_scene_fn_ = filename;
  }

private:
  std::string init_scene_fn_;
  std::shared_ptr<mineola::CameraController> cam_ctrl_;
};

}

int main(int argc, char *argv[]) {
  std::shared_ptr<mineola_pc::TestApp> app(new mineola_pc::TestApp());
  if (argc >= 2) {
    app->SetScenePath(argv[1]);
  }
  app->Run(512, 512);
  return 0;
}
