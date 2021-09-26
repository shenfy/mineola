#include <mineola/AppFrame.h>
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

static const std::string kSceneFilename = "ocean.json";
static const float PI = 3.1415926535897932395627f;

}

namespace mineola {

auto CreateGeometry(
  const std::vector<glm::vec3> &positions,
  const std::vector<glm::vec3> &normals,
  const std::vector<glm::ivec3> &triangles,
  vertex_type::VertexArray &vertex_array) {

  using namespace mineola::vertex_type;

  auto num_verts = (uint32_t)positions.size();
  auto num_triangles = (uint32_t)triangles.size();

  auto vs = std::make_shared<vertex_type::VertexStream>();
  auto ns = std::make_shared<vertex_type::VertexStream>();
  auto is = std::make_shared<vertex_type::VertexStream>();
  vs->layout.push_back({POSITION, type_mapping::FLOAT32, 3});
  vs->type = VST_VERTEX;
  vs->size = num_verts;
  vs->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STREAM, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  vs->buffer_ptr->Bind();
  vs->buffer_ptr->SetData(vs->Stride() * vs->size, glm::value_ptr(positions[0]));

  ns->layout.push_back({NORMAL, type_mapping::FLOAT32, 3});
  ns->type = VST_VERTEX;
  ns->size = num_verts;
  ns->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STREAM, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY, GL_ARRAY_BUFFER);
  ns->buffer_ptr->Bind();
  ns->buffer_ptr->SetData(ns->Stride() * ns->size, glm::value_ptr(normals[0]));

  is->layout.push_back({INDEX, type_mapping::UINT32, 1});
  is->type = VST_INDEX;
  is->size = num_triangles * 3;
  is->buffer_ptr = std::make_shared<GraphicsBuffer>(
    GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::READ_ONLY,
    GL_ELEMENT_ARRAY_BUFFER);
  is->buffer_ptr->Bind();
  is->buffer_ptr->SetData(is->Stride() * is->size, glm::value_ptr(triangles[0]));

  vertex_array.PrimitiveType() = GL_TRIANGLES;
  vertex_array.AddVertexStream(vs);
  vertex_array.AddVertexStream(ns);
  vertex_array.SetIndexStream(is);

  return std::tuple(vs, ns);
}


class WaterSurface : public Entity {
public:
  WaterSurface() = default;
  ~WaterSurface() override = default;

  static int Loc(int x, int y, int width) {
    return y * width + x;
  }

  void InitCPUData(const glm::ivec2 &num_segments) {

    float sqrt_1_2 = std::sqrt(2.f) * .5f;

    auto num_verts_x = num_segments.x + 1;
    auto num_verts_y = num_segments.y + 1;
    auto num_verts = num_verts_x * num_verts_y;

    positions_.resize(num_verts);
    auto size = glm::vec3(lattice_size_ * num_segments.x, 0.f, lattice_size_ * num_segments.y);
    auto start_pos = -size * 0.5f;

    for (int y = 0; y < num_verts_y; y++) {
      for (int x = 0; x < num_verts_x; x++) {
        positions_[y * num_verts_x + x] =
          start_pos + glm::vec3(lattice_size_ * x, 0.f, lattice_size_ * y);
      }
    }

    normals_ = std::move(std::vector<glm::vec3>(num_verts, glm::vec3(0.f, 1.f, 0.f)));

    triangles_.clear();
    for (int y = 0; y < num_segments.y; y++) {
      for (int x = 0; x < num_segments.x; x++) {
        auto v00 = y * num_verts_x + x;
        auto v10 = y * num_verts_x + x + 1;
        auto v01 = (y + 1) * num_verts_x + x;
        auto v11 = (y + 1) * num_verts_x + (x + 1);
        triangles_.push_back(glm::vec3{v00, v10, v11});
        triangles_.push_back(glm::vec3{v00, v11, v01});
      }
    }

    ori_pos_.clear();
    ori_pos_.reserve(positions_.size());
    std::copy(positions_.begin(), positions_.end(), std::back_inserter(ori_pos_));

    levels_.clear();
    std::srand(std::time(nullptr));
    for (int i = 1; i <= 20; i += 2) {
      Level lvl;
      lvl.wave_number = (float)i;
      lvl.theta = (float)std::rand() / RAND_MAX * 2 * PI;
      lvl.A = 0.3f * std::exp(-lvl.wave_number * 0.4);
      lvl.phase = (float)std::rand() / RAND_MAX * PI;
      levels_.push_back(lvl);
    }
  }

  auto InitGPUData() {
    auto va = std::make_shared<vertex_type::VertexArray>();
    std::tie(vs_pos_, vs_n_) = CreateGeometry(positions_, normals_, triangles_, *va);

    return va;
  }

  auto InitData(
    float lattice_size, const glm::ivec2 &num_segments) {

    num_segments_ = num_segments;
    lattice_size_ = lattice_size;
    InitCPUData(num_segments_);

    return InitGPUData();
  }

  void UpdateCPUData(float cur_time) {

    int num_verts_x = num_segments_.x + 1;
    int num_verts_y = num_segments_.y + 1;

    for (int z = 0; z < num_verts_y; z++) {
      for (int x = 0; x < num_verts_x; x++) {
        int loc = Loc(x, z, num_verts_x);
        positions_[loc] = ori_pos_[loc];
      }
    }

    for (const auto &level : levels_) {
      float k = level.wave_number;
      auto dir_n = glm::normalize(glm::vec3{std::cos(level.theta), 0.f, std::sin(level.theta)});
      auto dir = dir_n * k;
      auto omega = std::sqrt(1.0f * k);

      // calculate Gerstner wave
      for (int z = 0; z < num_verts_y; z++) {
        for (int x = 0; x < num_verts_x; x++) {

          auto loc = Loc(x, z, num_verts_x);
          auto phase = dir.x * ori_pos_[loc].x + dir.z * ori_pos_[loc].z
            - omega * cur_time / 1000.0f + level.phase;

          positions_[loc] -= dir_n * level.A * std::sin(phase);
          positions_[loc] += glm::vec3(0.f, 1.f, 0.f) * level.A * std::cos(phase);
        }
      }
    }

    // recalculate normal
    for (auto &n : normals_) {
      n = glm::vec3(0.f);
    }
    for (const auto &t : triangles_) {
      auto n = -glm::normalize(glm::cross(
        positions_[t[1]] - positions_[t[0]],
        positions_[t[2]] - positions_[t[1]]));
      normals_[t[0]] += n;
      normals_[t[1]] += n;
      normals_[t[2]] += n;
    }
    for (auto &n : normals_) {
      n = glm::normalize(n);
    }
  }

  void UpdateGPUData() {
    // update pos buffer
    auto vs_p = vs_pos_.lock();
    vs_p->buffer_ptr->Bind();
    vs_p->buffer_ptr->UpdateData(0, vs_p->Stride() * vs_p->size, glm::value_ptr(positions_[0]));

    auto vs_n = vs_n_.lock();
    vs_n->buffer_ptr->Bind();
    vs_n->buffer_ptr->UpdateData(0, vs_n->Stride() * vs_n->size, glm::value_ptr(normals_[0]));
  }

  void FrameMove(double time, double frame_time) override {
    float total_time = (float)frame_time / 1000.f;  // s

    UpdateCPUData(time);

    UpdateGPUData();
  }

  void OnKey(uint32_t key, uint32_t action) {
    if (action == Engine::BUTTON_UP) {
      auto &en = Engine::Instance();
      if (key == 'Q') {
        en.SignalTermination();
      } else if (key == 'R') {
        InitCPUData(num_segments_);
      }
    }
  }

private:
  glm::ivec2 num_segments_;
  std::weak_ptr<vertex_type::VertexStream> vs_pos_;
  std::weak_ptr<vertex_type::VertexStream> vs_n_;
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> ori_pos_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::ivec3> triangles_;

  float lattice_size_{0.1f};

  struct Level {
    float wave_number{1.0f};
    float theta{0.f};
    float A{0.1f};
    float phase{0.f};
  };
  std::vector<Level> levels_;
};

class WaterWaveApp : public AppFrame,
  public std::enable_shared_from_this<WaterWaveApp> {
public:
  WaterWaveApp() {}
  virtual ~WaterWaveApp() {}

  virtual bool InitScene() override {
    AppFrame::InitScene();

    Engine &en = GetEngine();

    en.SetExtTextureLoaders(STBLoadImageFromFile, STBLoadImageFromMem);
    en.ResrcMgr().AddSearchPath("resrc");

    BuildSceneFromConfigFile(kSceneFilename.c_str(), {
      std::bind(gltf::LoadScene,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5,
        false)
    });

    // add geometry
    auto water = std::make_shared<WaterSurface>();
    auto va = water->InitData(0.02f, {400, 500});
    en.EntityMgr().Add("water", bd_cast<Entity>(water));

    auto renderable = std::make_shared<Renderable>();
    renderable->AddVertexArray(va, "mat:water");
    renderable->SetEffect("effect:ocean");

    auto geometry_node = SceneNode::FindNodeByName("geometry", en.Scene().get());
    if (geometry_node) {
      geometry_node->Renderables().push_back(renderable);
    }

    en.AddKeyboardCallback([water](uint32_t key, uint32_t action) {
      water->OnKey(key, action);
    });

    en.ChangeCamera("main", false);

    return true;
  }
};

}

int main(int argc, char *argv[]) {
  std::shared_ptr<mineola::WaterWaveApp> app(new mineola::WaterWaveApp());
  app->Run(700, 512);
  return 0;
}
