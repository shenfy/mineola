#include "../include/AppFrame.h"
#include <memory>
#include <iostream>
#include "../../include/AppHelper.h"
#include "../../include/CameraController.h"
#include "../../include/SceneLoader.h"
#include "../../include/Material.h"
#include "../../include/MeshIO.h"
#include "../../include/FileSystem.h"
#include "../../include/KTXImageLoader.h"
#include "../../include/ImgppTextureSrc.h"
#include "../../include/TextureHelper.h"
#define MINEOLA_USE_STBIMAGE
#include "../../include/STBImagePlugin.h"

#include "../include/TextureMixer.h"

namespace {
const char kTexFn[] = "resrc/kueken7_rgba_astc4x4_srgb.ktx";
const char kTexName[] = "texture0";
}

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
    en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);
    original_src_ = LoadKTXFromFile(kTexFn);
    if (!original_src_) {
      std::cout << "Failed to load tex: " << kTexFn << std::endl;
      return false;
    }

    auto &roi_0 = original_src_->BCROI(0);
    dst_src_ = std::make_shared<ImgppTextureSrc>(1, 1, 1);
    imgpp::BCImg img(roi_0.Format(), roi_0.Width(), roi_0.Height());
    imgpp::CopyData(img.ROI(), roi_0);
    dst_src_->AddBuffer(img.Data());
    dst_src_->AddBCROI(img.ROI());

    TextureDesc desc;
    if (!texture_helper::CreateTextureDescFromImgppTextureSrc(dst_src_, true, false, desc)) {
      std::cout << "Failed to create desc" << std::endl;
      return false;
    }
    if (!texture_helper::CreateTextureFromDesc(kTexName, desc)) {
      std::cout << "Failed to create texture" << std::endl;
      return false;
    }
    if (!init_scene_fn_.empty()) {
      BuildSceneFromConfigFile(init_scene_fn_.c_str(), {mesh_io::LoadPLY});
    }

    cam_ctrl_.reset(new TurntableController);
    // cam_ctrl_.reset(new ArcballController);
    cam_ctrl_->BindToNode("camera");
    cam_ctrl_->Activate();
    bd_cast<TurntableController>(cam_ctrl_)->SetSpeed(0.1f);
    bd_cast<TurntableController>(cam_ctrl_)->SetTarget("geometry");


    en.RenderStateMgr().SetClearColor(glm::vec4(0.7f, 0.4f, 0.8f, 0.0f), true);
    en.ChangeCamera("main", false);

    auto tex_mixer = std::make_shared<TextureMixer>(kTexName, roi_0, img.ROI());
    en.EntityMgr().Add("entity:texmixer", tex_mixer);
    return true;
  }

  void SetScenePath(const char *filename) {
    init_scene_fn_ = filename;
  }

private:
  std::string init_scene_fn_;
  std::shared_ptr<mineola::CameraController> cam_ctrl_;
  std::shared_ptr<ImgppTextureSrc> original_src_;
  std::shared_ptr<ImgppTextureSrc> dst_src_;
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
