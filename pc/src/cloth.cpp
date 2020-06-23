#include "../include/AppFrame.h"
#include <memory>
#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/AppHelper.h>
#include <mineola/CameraController.h>
#include <mineola/SceneLoader.h>
#include <mineola/MeshIO.h>
#include <mineola/FileSystem.h>
#include <mineola/GraphicsBuffer.h>
#include <mineola/Renderable.h>
#include <mineola/SceneNode.h>
#include <mineola/Entity.h>
#define MINEOLA_USE_STBIMAGE
#include <mineola/GLTFLoader.h>
#include <mineola/STBImagePlugin.h>

namespace {

static const std::string kSceneFilename = "cloth.json";

}

namespace mineola {

std::shared_ptr<vertex_type::VertexStream> CreateGeometry(
  const std::vector<glm::vec3> &positions,
  const std::vector<glm::vec2> &uvs,
  const std::vector<glm::ivec3> &triangles,
  vertex_type::VertexArray &vertex_array) {

  using namespace mineola::vertex_type;

  auto num_verts = (uint32_t)positions.size();
  auto num_triangles = (uint32_t)triangles.size();

  auto vs = std::make_shared<vertex_type::VertexStream>();
  auto ts = std::make_shared<vertex_type::VertexStream>();
  auto is = std::make_shared<vertex_type::VertexStream>();
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = num_verts;
  vs->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STREAM, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, glm::value_ptr(positions[0]));

  ts->layout.push_back({TEXCOORD0, type_mapping::FLOAT32, 2});
  ts->type = VST_VERTEX;
  ts->size = num_verts;
  ts->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  ts->buffer_ptr->Bind();
  ts->buffer_ptr->SetData(ts->Stride() * ts->size, glm::value_ptr(uvs[0]));

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = num_triangles * 3;
  is->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER);
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, glm::value_ptr(triangles[0]));

  vertex_array.AddVertexStream(vs);
  vertex_array.AddVertexStream(ts);
  vertex_array.SetIndexStream(is);

  return vs;
}


class Cloth : public Entity {
public:
  Cloth() = default;
  ~Cloth() override = default;

  static int Loc(int x, int y, int width) {
    return y * width + x;
  }

  auto InitCPUData(const glm::ivec2 &num_segments) {

    float sqrt_1_2 = std::sqrt(2.f) * .5f;
    auto d_pos = glm::vec3(rest_len_, sqrt_1_2 * rest_len_, sqrt_1_2 * rest_len_);
    auto d_uv = glm::vec2(1.f / num_segments.x, 1.f / num_segments.y);

    auto num_verts_x = num_segments.x + 1;
    auto num_verts_y = num_segments.y + 1;
    auto num_verts = num_verts_x * num_verts_y;

    std::vector<glm::vec3> positions(num_verts);
    std::vector<glm::vec2> uvs(num_verts);

    for (int y = 0; y < num_verts_y; y++) {
      for (int x = 0; x < num_verts_x; x++) {
        positions[y * num_verts_x + x] = glm::vec3(d_pos.x * x, d_pos.y * y, d_pos.z * y);
        uvs[y * num_verts_x + x] = glm::vec2(d_uv.x * x, d_uv.y * y);
      }
    }

    std::vector<glm::ivec3> triangles;
    for (int y = 0; y < num_segments.y; y++) {
      for (int x = 0; x < num_segments.x; x++) {
        auto v00 = y * num_verts_x + x;
        auto v10 = y * num_verts_x + x + 1;
        auto v01 = (y + 1) * num_verts_x + x;
        auto v11 = (y + 1) * num_verts_x + (x + 1);
        triangles.push_back(glm::vec3{v00, v10, v11});
        triangles.push_back(glm::vec3{v00, v11, v01});
      }
    }

    return std::make_tuple(positions, uvs, triangles);
  }

  std::shared_ptr<vertex_type::VertexArray> InitData(
    float rest_len, const glm::ivec2 &num_segments) {

    num_segments_ = num_segments;
    rest_len_ = rest_len;
    auto [positions, uvs, triangles] = InitCPUData(num_segments_);
    positions_ = std::move(positions);

    auto va = std::make_shared<vertex_type::VertexArray>();
    vs_ = CreateGeometry(positions_, uvs, triangles, *va);

    auto num_verts = positions_.size();
    forces_.resize(num_verts);
    v_.resize(num_verts);

    ClearVelocity();

    return va;
  }

  void UpdateCPUData(float d_time) {
    static const glm::vec3 G{0.f, -9.8f, 0.f};

    int num_verts_x = num_segments_.x + 1;
    int num_verts_y = num_segments_.y + 1;

    static const glm::ivec2 dirs[] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    // calculate force (ie. acc=dv/dt=)
    ClearForces();
    for (int y = 0; y < num_verts_y; y++) {
      for (int x = 0; x < num_verts_x; x++) {
        for (int d = 0; d < 4; d++) {
          int nx = x + dirs[d].x;
          int ny = y + dirs[d].y;
          if (nx < 0 || nx >= num_verts_x || ny < 0 || ny >= num_verts_y) {
            continue;
          }

          auto &p0 = positions_[Loc(x, y, num_verts_x)];
          auto &p1 = positions_[Loc(nx, ny, num_verts_x)];
          auto new_dist = glm::distance(p0, p1);
          auto length_diff = new_dist - rest_len_;
          auto loc = Loc(x, y, num_verts_x);
          forces_[loc] += (k_ * length_diff) * (p1 - p0);

          // gravity
          forces_[loc] += G * mass_;
        }
      }
    }

    // calculate v, x
    for (int y = 0; y < num_verts_y; y++) {
      for (int x = 0; x < num_verts_x; x++) {
        auto loc = Loc(x, y, num_verts_x);
        if (y == num_verts_y - 1 || x % 4 == 0) {  // top row fixed
          v_[loc] = glm::vec3(0.f, 0.f, 0.f);
          continue;
        }
        v_[loc] += (forces_[loc] - damp_ * v_[loc]) / mass_ * d_time;

        positions_[loc] += v_[loc] * d_time;
      }
    }
  }

  void UpdateGPUData() {
    // update pos buffer
    auto vs = vs_.lock();
    vs->buffer_ptr->Bind();
    vs->buffer_ptr->UpdateData(0, vs->Stride() * vs->size, glm::value_ptr(positions_[0]));
  }

  void FrameMove(double time, double frame_time) override {
    if (!running_) {
      return;
    }

    float total_time = (float)frame_time / 1000.f;  // s

    int steps = (int)(total_time / time_step_);
    if (steps > 0) {
      for (int i = 0; i < steps; i++) {
        UpdateCPUData(time_step_);
      }
    }

    float remaining_time = total_time - time_step_ * steps;
    if (remaining_time > 0.f) {
      UpdateCPUData(remaining_time);
    }

    UpdateGPUData();
  }

  void OnKey(uint32_t key, uint32_t action) {
    if (action == Engine::BUTTON_UP) {
      auto &en = Engine::Instance();
      if (key == 'Q') {
        en.SignalTermination();
      } else if (key == 'P') {
        PlayStop();
      } else if (key == 'R') {
        Reset();
      }
    }
  }

  void PlayStop() {
    running_ = !running_;
  }

  void Reset() {
    running_ = false;
    auto [positions, uvs, triangles] = InitCPUData(num_segments_);
    positions_ = std::move(positions);
    ClearVelocity();
    UpdateGPUData();
  }

private:
  void ClearForces() {
    auto num_verts = (num_segments_.x + 1) * (num_segments_.y + 1);
    memset(glm::value_ptr(forces_[0]), 0, sizeof(glm::vec3) * num_verts);
  }

  void ClearVelocity() {
    auto num_verts = (num_segments_.x + 1) * (num_segments_.y + 1);
    memset(glm::value_ptr(v_[0]), 0, sizeof(glm::vec3) * num_verts);
  }

  glm::ivec2 num_segments_;
  std::weak_ptr<vertex_type::VertexStream> vs_;
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> forces_;
  std::vector<glm::vec3> v_;
  bool running_{false};

  float rest_len_;
  float k_{980.0f * 3.f};  // spring stiffness
  float damp_{.5f};  // damper
  float mass_{1.f};
  float time_step_{2e-3f};  // 2ms
};

class MassSpringApp : public AppFrame,
  public std::enable_shared_from_this<MassSpringApp> {
public:
  MassSpringApp() {}
  virtual ~MassSpringApp() {}

  virtual bool InitScene() override {
    AppFrame::InitScene();

    Engine &en = GetEngine();

    en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);
    en.ResrcMgr().AddSearchPath("resrc");

    BuildSceneFromConfigFile(kSceneFilename.c_str(), {gltf::LoadScene});

    // add geometry
    auto cloth = std::make_shared<Cloth>();
    auto va = cloth->InitData(1.f, {10, 10});
    en.EntityMgr().Add("cloth", bd_cast<Entity>(cloth));

    auto renderable = std::make_shared<Renderable>();
    renderable->AddVertexArray(va, "mat:cloth");
    renderable->SetEffect("mineola:effect:ambienttex");

    auto geometry_node = SceneNode::FindNodeByName("geometry", en.Scene().get());
    if (geometry_node) {
      geometry_node->Renderables().push_back(renderable);
    }

    cam_ctrl_.reset(new ArcballController);
    cam_ctrl_->BindToNode("camera");
    cam_ctrl_->Activate();
    bd_cast<ArcballController>(cam_ctrl_)->SetSpeed(0.1f);

    en.AddKeyboardCallback([cloth](uint32_t key, uint32_t action) {
      cloth->OnKey(key, action);
    });

    en.ChangeCamera("main", false);

    return true;
  }

private:
  std::shared_ptr<CameraController> cam_ctrl_;
};

}

int main(int argc, char *argv[]) {
  std::shared_ptr<mineola::MassSpringApp> app(new mineola::MassSpringApp());
  app->Run(512, 512);
  return 0;
}
