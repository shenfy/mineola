#include <mineola/GuiAppFrame.h>
#include <memory>
#include <iostream>
#include <imgui.h>
#include <mineola/AppHelper.h>
#include <mineola/CameraController.h>
#include <mineola/SceneNode.h>
#include <mineola/SceneLoader.h>
#include <mineola/MeshIO.h>
#include <mineola/Material.h>

namespace {

const std::string kSceneFn = "guidemo.json";

}

namespace mineola_pc {
using namespace mineola;

class GUIEntity : public Entity {
public:
  GUIEntity() = default;
  ~GUIEntity() override = default;

  void RenderGUI() override {
    ImGui::SetNextWindowSize({200, 120});
    ImGui::SetNextWindowPos({20, 20});
    ImGui::Begin("GUI Demo");
    ImGui::Text("Hello GUI!");
    ImGui::SliderFloat("float", &f_value_, -1.0f, 1.0f);
    ImGui::SliderInt("int", &i_value_, 0, 100);
    if (ImGui::Button("Exit")) {
      GetEngine().SignalTermination();
    }
    ImGui::End();

    ImGui::SetNextWindowSize({200, 80});
    ImGui::SetNextWindowPos({20, 160});
    ImGui::Begin("GUI Panel 2");
    ImGui::Text("Second Window");
    ImGui::ColorEdit3("Color", &color_[0]);
    ImGui::End();
  }

  void FrameMove(double time, double frame_time) override {
    auto &en = GetEngine();
    auto mat = bd_cast<Material>(en.ResrcMgr().Find("mat:sphere"));
    mat->diffuse = color_;
  }

private:
  float f_value_{0.0f};
  int i_value_{0};
  glm::vec3 color_{1.0f, 1.0f, 1.0f};
};

class GuiDemoApp : public mineola::GuiAppFrame,
  public std::enable_shared_from_this<GuiDemoApp> {
public:
  GuiDemoApp() {}
  virtual ~GuiDemoApp() {}

  bool InitScene() override {
    GuiAppFrame::InitScene();

    Engine &en = GetEngine();
    en.ResrcMgr().AddSearchPath("./resrc");

    BuildSceneFromConfigFile(kSceneFn.c_str(), {});

    auto entity = std::make_shared<GUIEntity>();
    en.EntityMgr().Add("entity:gui", bd_cast<Entity>(entity));

    // camera control
    cam_ctrl_ = std::make_shared<ArcballController>();
    cam_ctrl_->BindToNode("camera");
    cam_ctrl_->Activate();

    en.ChangeCamera("camera:main", false);

    return true;
  }

private:
  std::shared_ptr<mineola::CameraController> cam_ctrl_;
};

}

int main(int argc, char *argv[]) {
  auto app = std::make_shared<mineola_pc::GuiDemoApp>();
  app->Run(800, 800);
  return 0;
}
