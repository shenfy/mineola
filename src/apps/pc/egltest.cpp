#define GLM_FORCE_CTOR_INIT
#include <mineola/ServerAppFrame.h>
#include <memory>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/GraphicsBuffer.h>
#include <mineola/TextureHelper.h>
#include <mineola/Framebuffer.h>
#include <mineola/PrimitiveHelper.h>
#include <mineola/Engine.h>
#include <mineola/Renderable.h>
#include <mineola/SceneNode.h>
#include <mineola/Entity.h>
#include <mineola/ImgppLoaders.hpp>

class ScreenShotEntity: public mineola::Entity {
public:
  ScreenShotEntity() = default;
  ~ScreenShotEntity() override = default;

  void FrameMove(double time, double frame_time) override {
    using namespace mineola;

    static int count = 0;
    auto &en = Engine::Instance();
    auto fb =
      bd_cast<InternalFramebuffer>(en.ResrcMgr().Find("framebuffer:offscr"));
    fb->Bind();
    fb->ReadBack(Framebuffer::AT_COLOR0);
    GraphicsBuffer &buf = fb->GetReadPBO();
    uint8_t *p = (uint8_t*)buf.Map();
    mineola::imgpp::ImgROI roi(p, 512, 512, 3, 32, 4*512*3, true, true);
    mineola::imgpp::WritePFM("result.pfm", roi, true);
    buf.Unmap();

    count++;
    if (count == 6) {
      en.SignalTermination();
    }
  }
};

class ServerTestApp : public mineola::ServerAppFrame,
        public std::enable_shared_from_this<ServerTestApp> {
public:
  ServerTestApp() {}
  virtual ~ServerTestApp() {}

  virtual void InitScene() override {
    using namespace mineola;
    ServerAppFrame::InitScene();

    Engine &en = Engine::Instance();

    auto va1 = std::make_shared<vertex_type::VertexArray>();
    primitive_helper::BuildSphere(3, *va1);

    auto va2 = std::make_shared<vertex_type::VertexArray>();
    primitive_helper::BuildRect(2.0f, *va2);
    auto renderable = std::make_shared<Renderable>();
    renderable->AddVertexArray(va1, "mineola:material:fallback");
    renderable->AddVertexArray(va2, "mineola:material:fallback");
    renderable->SetEffect("mineola:effect:fallback");

    en.Scene()->Renderables().push_back(renderable);

    RenderPass render_pass;
    render_pass.override_render_target = "framebuffer:offscr";
    en.RenderPasses().push_back(render_pass);


    if (!texture_helper::CreateDepthTexture("texture:depth", 512, 512, 24, false)) {
      printf("Failed creating depth texture!\n");
      return;
    }
    if (!texture_helper::CreateEmptyTexture("texture:color_0",
      GL_TEXTURE_2D, 512, 512, 1, 1, 1, GL_RGB, GL_RGB32F, GL_FLOAT)) {
      printf("Failed creating texture!\n");
      return;
    }
    auto fb = std::make_shared<InternalFramebuffer>(1);
    fb->AttachTexture(Framebuffer::AT_COLOR0, "texture:color_0");
    fb->AttachTexture(Framebuffer::AT_DEPTH, "texture:depth");
    en.ResrcMgr().Add("framebuffer:offscr", bd_cast<Resource>(fb));

    // camera
    std::shared_ptr<Camera> camera(new Camera(true));
    camera->SetProjParams(glm::radians(60.0f), 0.01f, 100.f);
    en.CameraMgr().Add("camera:test", camera);
    en.ChangeCamera("camera:test", false);

    // camera node
    std::shared_ptr<SceneNode> camera_node(new SceneNode);
    camera_node->SetRbt(math::Rbt::LookAt(
      glm::vec3(0.0f,0.0f,4.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0,1,0)));
    SceneNode::LinkTo(camera_node, en.Scene());
    camera_node->Cameras().push_back(camera);
    camera_node->SetName("cam_node");

    // entity
    auto scrshot = std::make_shared<ScreenShotEntity>();
    en.EntityMgr().Add("scrshot", bd_cast<Entity>(scrshot));
  }

private:
};

int main() {
  std::shared_ptr<ServerTestApp> app(new ServerTestApp);
  app->Run(512, 512);
  return 0;
}
