#include "prefix.h"
#include <mineola/Engine.h>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <imgpp/imgpp.hpp>
#include <mineola/glutility.h>
#include <mineola/GLEffect.h>
#include <mineola/Framebuffer.h>
#include <mineola/TextureHelper.h>
#include <mineola/Material.h>
#include <mineola/Renderable.h>
#include <mineola/SceneNode.h>
#include <mineola/UniformBlock.h>
#include <mineola/Light.h>
#include <mineola/Viewport.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace mineola {

using RenderQueue = std::vector<std::pair<glm::mat4, std::shared_ptr<Renderable>>>;

namespace {
  bool RenderableLess(
    const std::pair<glm::mat4, std::shared_ptr<Renderable>> &r_a,
    const std::pair<glm::mat4, std::shared_ptr<Renderable>> &r_b) {
    return r_a.second->QueueId() < r_b.second->QueueId();
  }

  RenderQueue GenerateRenderQueue(const SceneNode &root) {
    RenderQueue result;
    root.DFTraverse([&result](const SceneNode &node) {
      // retrieve current world transform
      auto world_rbt = node.WorldRbt();
      auto world_scale = node.WorldScale();

      // add renderables to render list
      auto model_matrix = glm::scale(world_rbt.ToMatrix(), world_scale);
      for (auto &renderable : node.Renderables())
        result.push_back({model_matrix, renderable});
    });
    std::sort(result.begin(), result.end(), RenderableLess);
    return result;
  };
}

Engine::Engine()
  :current_viewport_(0),
  time_(0.0), frame_time_(0.0),
  override_effect_(false),
  override_camera_(false),
  override_render_target_(false),
  ext_texture_loader_(nullptr),
  ext_texture_mem_loader_(nullptr),
  terminate_signaled_(false) {
  timer_.reset(new Timer);
  timer_->Start();

  root_node_ = std::make_shared<SceneNode>();
}

Engine::~Engine() {}

Engine &Engine::Instance() {
  static Engine engine;
  return engine;
}

ResourceManager &Engine::ResrcMgr() {
  return resrc_mgr_;
}

RenderStateManager &Engine::RenderStateMgr() {
  return render_state_mgr_;
}

ManagerBase<Entity> &Engine::EntityMgr() {
  return entity_mgr_;
}

ManagerBase<Camera> &Engine::CameraMgr() {
  return camera_mgr_;
}

void Engine::ChangeEffect(const std::string &name, bool force) {

  if (override_effect_)  // lock effect in override mode
    return;

  std::shared_ptr<GLEffect> p;
  if (name.length() == 0) {  //load default fallback effect
    p = bd_cast<GLEffect>(resrc_mgr_.Find("mineola:effect:fallback"));
    current_effect_.first = "mineola:effect:fallback";
    current_effect_.second = p;
  }
  else if (!force && current_effect_.first == name) //same effect, do nothing
    return;
  else {
    p = bd_cast<GLEffect>(resrc_mgr_.Find(name));
    if (p) {
      current_effect_.first = name;
      current_effect_.second = p;
    } else {
      p = bd_cast<GLEffect>(resrc_mgr_.Find("mineola:effect:fallback"));
      current_effect_.first = "mineola:effect:fallback";
      current_effect_.second = p;
    }
  }
  p->ApplyRenderStates();
  p->Bind();

  //upload active camera info
  if (current_camera_.second)
    current_camera_.second->Activate();
}

std::shared_ptr<GLEffect> &Engine::CurrentEffect() {
  return current_effect_.second;
}

void Engine::ChangeCamera(const std::string &name, bool force) {
  if (override_camera_)
    return;

  if (name.length() == 0 || (!force && current_camera_.first == name))
    return;

  std::shared_ptr<Camera> cam = bd_cast<Camera>(camera_mgr_.Find(name));
  if (cam) {
    current_camera_.first = name;
    current_camera_.second = cam;
    if (current_framebuffer_.second) {
        auto &vp = current_framebuffer_.second->GetViewport(current_viewport_);
        cam->OnSize(vp.get());
    }
    cam->Activate();
  }
}

std::shared_ptr<Camera> &Engine::CurrentCamera() {
  return current_camera_.second;
}

void Engine::SetDefaultFramebuffer(uint32_t fbo, uint32_t width, uint32_t height) {
  auto scr = scr_framebuffer_.lock();
  if (scr) {
    auto efb = bd_cast<ExternalFramebuffer>(scr);
    if (efb) {
      efb->SetParams(fbo, width, height);
      SetFramebuffer("mineola:framebuffer:screen", true);
    }
  }
}

void Engine::SetFramebuffer(const char *name, bool force, uint32_t viewport) {
  if (override_render_target_)  // lock render target
    return;

  CHKGLERR

  if (name == NULL || strlen(name) == 0
    || (!force && current_framebuffer_.first == name))
    return;

  std::shared_ptr<Framebuffer> p = bd_cast<Framebuffer>(resrc_mgr_.Find(name));
  if (p) {
    current_framebuffer_.first = name;
    current_framebuffer_.second = p;
    p->SetAsRenderTarget();
    SetViewport(viewport);
    CHKGLERR
  }

  CHKGLERR
}

std::shared_ptr<Framebuffer> &Engine::CurrentFramebuffer() {
  return current_framebuffer_.second;
}

std::shared_ptr<Framebuffer> Engine::GetScrFramebuffer() {
  return scr_framebuffer_.lock();
}

void Engine::SetViewport(uint32_t id) {
  std::shared_ptr<Framebuffer> &fb = current_framebuffer_.second;
  if (fb && id < fb->NumViewport()) {
    auto &vp = fb->GetViewport(id);
    vp->Activate();
    if (current_camera_.second) {
      current_camera_.second->OnSize(vp.get());
      current_camera_.second->Activate();
    }
    current_viewport_ = id;
  }
}

void Engine::OnSize(uint32_t width, uint32_t height) {
   //resize on-screen framebuffer
  const std::shared_ptr<Framebuffer> &framebuffer =
    bd_cast<Framebuffer>(resrc_mgr_.Find("mineola:framebuffer:screen"));
  if (framebuffer) {
    uint32_t num_viewports = framebuffer->NumViewport();
    for (uint32_t i = 0; i < num_viewports; ++i) {
      framebuffer->GetViewport(i)->OnSize(width, height);
    }

    if (current_framebuffer_.second == framebuffer)
      SetViewport(current_viewport_);
  }

  size_change_sig_(width, height);
}

double Engine::LastFrameTime() {
  return time_;
}

double Engine::Now() {
  return timer_->Snapshot();
}

void Engine::FrameMove() {
  //calc time since last frame
  static bool bFirstTime = true;
  if (bFirstTime) {
    timer_->Start();
    bFirstTime = false;
  }
  double now = timer_->Snapshot();
  frame_time_ = now - time_;

  frame_move_sig_(now, frame_time_);

  entity_mgr_.Transform(
    [now, this](const std::string &, std::shared_ptr<Entity> &entity) {
      entity->FrameMove(now, frame_time_);
  });

  time_ = now;
  auto builtin_ub = builtin_uniform_block_.lock();
  if (builtin_ub) {
    builtin_ub->UpdateVariable("_time", glm::value_ptr(glm::vec4((float)time_)));
    builtin_ub->UpdateVariable("_delta_time", glm::value_ptr(glm::vec4((float)frame_time_)));
  }

  // update scenenode tree
  root_node_->UpdateSubtreeWorldTforms();
}

void Engine::Render() {

   // call entity prerender
  entity_mgr_.Transform([](const std::string &, std::shared_ptr<Entity> &entity) {
    entity->PreRender();
  });

  auto builtin_ub = builtin_uniform_block_.lock();
  if (builtin_ub) {
    // activate builtin uniform block
    builtin_ub->Activate();
    // update light uniforms
    root_node_->DFTraverse([&builtin_ub](SceneNode &node) {
      for (auto &light : node.Lights()) {
        light->UpdateUniforms(builtin_ub.get());
      }
    });
  }

  // clear current camera, effect, framebuffer at the beginning of a pass
  current_camera_.first.clear();
  current_effect_.first.clear();

  if (current_camera_.second)
    current_camera_.second->Activate();

  // refresh scene tree and generate render list
  RenderQueue render_queue = GenerateRenderQueue(*root_node_);
  // cache last non-override camera and render target
  std::string previous_camera = "";
  bool need_restore_camera = false;
  std::string previous_rt = "";
  bool need_restore_rt = false;

  // loop over all passes
  for (uint32_t pass_idx = 0; pass_idx < (uint32_t)render_passes_.size(); ++pass_idx) {
    const auto &pass = render_passes_[pass_idx];

    pass_begin_sig_(pass_idx);

    if (!pass.override_effect.empty()) {
      override_effect_ = false;  // unlock effect
      ChangeEffect(pass.override_effect, false);
      override_effect_ = true;  // lock effect
    } else {
      override_effect_ = false;
    }

    if (!pass.override_camera.empty()) {
      if (!need_restore_camera) {  // store non-override camera
        if (current_camera_.first.empty()) {
          previous_camera = camera_mgr_.QueryName(current_camera_.second);
        } else {
          previous_camera = current_camera_.first;
        }
        need_restore_camera = true;
      }
      override_camera_ = false;  // unlock camera
      ChangeCamera(pass.override_camera, false);
      override_camera_ = true;  // lock camera
    } else {
      override_camera_ = false;
      if (need_restore_camera) {  // restore the last non-override camera
        ChangeCamera(previous_camera, false);
        need_restore_camera = false;
      }
    }

    if (!pass.override_render_target.empty()) {
      if (!need_restore_rt) {
        if (current_framebuffer_.first.empty()) {
          previous_rt = resrc_mgr_.QueryName(current_framebuffer_.second);
        } else {
          previous_rt = current_framebuffer_.first;
        }
        need_restore_rt = true;
      }
      override_render_target_ = false;  // unlock render target
      SetFramebuffer(pass.override_render_target.c_str(), false);
      override_render_target_ = true;  // lock render target
    } else {
      override_render_target_ = false;
      if (need_restore_rt) {
        SetFramebuffer(previous_rt.c_str(), false);
        need_restore_rt = false;
      }
    }

    if (!pass.override_material.empty()) {
      override_material_ = pass.override_material;
    } else {
      override_material_.clear();
    }

    CHKGLERR

    int clear_flag = (pass.clear_flag & RenderPass::CLEAR_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0;
    clear_flag |= (pass.clear_flag & RenderPass::CLEAR_COLOR) ? GL_COLOR_BUFFER_BIT : 0;
    glClear(clear_flag);

    CHKGLERR

    // actual rendering
    for (auto iter = render_queue.begin(); iter != render_queue.end(); ++iter) {
      if (!(pass.layer_mask & iter->second->LayerMask()))  // skip masked objects
        continue;

      CHKGLERR
      iter->second->PreRender(frame_time_, pass_idx);
      current_effect_.second->UploadVariable("_model_mat", glm::value_ptr(iter->first));
      iter->second->Draw(frame_time_, pass_idx);
    }

    pass_end_sig_(pass_idx);
  }

  // unlock effect, camera, rt for user modifications
  override_effect_ = false;
  override_camera_ = false;
  override_render_target_ = false;

  if (need_restore_rt)
    SetFramebuffer(previous_rt.c_str(), false);
  if (need_restore_camera)
    ChangeCamera(previous_camera.c_str(), false);

  entity_mgr_.Transform([](const std::string &, std::shared_ptr<Entity> &entity) {
  	entity->PostRender();
  });
}

// create default framebuffer, etc.
void Engine::Init() {
  // create an external framebuffer with unintialized size
  auto framebuffer = std::make_shared<ExternalFramebuffer>();
  resrc_mgr_.Add("mineola:framebuffer:screen", bd_cast<Resource>(framebuffer));
  scr_framebuffer_ = framebuffer;

  texture_helper::CreateFallbackTexture2D();

  auto uniform_block = std::make_shared<UniformBlock>(
    UniformBlock::GetSemanticsBindLocation(UniformBlock::BUILTIN_UNIFORMS));
  // Note we're using std140 layout for builtin uniform buffer to save the effort of querying
  // member layout.
  // Make sure the offsets in SetVariable calls are correctly padded
  static const std::vector<std::pair<std::string, uint32_t>> builtin_uniforms = {
    {"_view_mat", 16 * (uint32_t)sizeof(float)},
    {"_view_mat_inv", 16 * (uint32_t)sizeof(float)},
    {"_proj_mat", 16 * (uint32_t)sizeof(float)},
    {"_proj_mat_inv", 16 * (uint32_t)sizeof(float)},
    {"_proj_view_mat", 16 * (uint32_t)sizeof(float)},
    {"_light_view_mat_0", 16 * (uint32_t)sizeof(float)},
    {"_light_proj_mat_0", 16 * (uint32_t)sizeof(float)},
    {"_viewport_size", 4 * (uint32_t)sizeof(float)},
    {"_light_pos_0", 4 * (uint32_t)sizeof(float)},
    {"_light_intensity_0", 4 * (uint32_t)sizeof(float)},
    {"_env_light_mat_0", 16 * (uint32_t)sizeof(float)},
    {"_env_light_sh3_0[0]", 9 * 4 * (uint32_t)sizeof(float)},
    {"_time", 4 * (uint32_t)sizeof(float)},
    {"_delta_time", 4 * (uint32_t)sizeof(float)},
  };
  uint32_t size = 0;
  for (auto &pair : builtin_uniforms) {
    uniform_block->SetVariable(pair.first.c_str(), size, pair.second);
    size += pair.second;
  }
  uniform_block->SetSize(size);
  resrc_mgr_.Add("mineola:uniformblock:builtin", bd_cast<Resource>(uniform_block));
  builtin_uniform_block_ = uniform_block;
  render_state_mgr_.ApplyCurrentState();

  #ifdef MINEOLA_LOG_TO_FILE
  log_.open("mineola.log", std::ios::app);
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  log_ << "\n=======================\nEngine started at " << std::ctime(&time);
  #endif
}

void Engine::Start() {
	entity_mgr_.Transform([](const std::string &, std::shared_ptr<Entity> &entity) {
		entity->Start();
	});
}

void Engine::Release() {
  root_node_ = std::make_shared<SceneNode>();
  entity_mgr_.Transform([](const std::string &, std::shared_ptr<Entity> &entity) {
  	entity->Destroy();
  });
  entity_mgr_.ReleaseResources();
  resrc_mgr_.ReleaseResources();
  camera_mgr_.ReleaseResources();
  current_effect_ = {"", nullptr};
  current_camera_ = {"", nullptr};
  current_framebuffer_ = {"", nullptr};
  current_viewport_ = 0;
  scr_framebuffer_.reset();

  #ifdef MINEOLA_LOG_TO_FILE
  log_.close();
  #endif
}

boost::signals2::connection Engine::AddKeyboardCallback(const keyboard_callback_t &callback) {
  return keyboard_sig_.connect(callback);
}

void Engine::OnKey(uint32_t key, uint8_t action) {
  keyboard_sig_(key, action);
}

boost::signals2::connection Engine::AddMouseButtonCallback(const mouse_btn_callback_t &callback) {
  return mouse_button_sig_.connect(callback);
}

void Engine::OnMouseButton(uint8_t button, uint8_t action, int x, int y) {
  mouse_button_sig_(button, action, x, y);
}

boost::signals2::connection Engine::AddMouseMoveCallback(const mouse_move_callback_t &callback) {
  return mouse_move_sig_.connect(callback);
}

void Engine::OnMouseMove(int x, int y) {
  mouse_move_sig_(x, y);
}

boost::signals2::connection Engine::AddPinchCallback(const pinch_callback_t &callback) {
  return pinch_sig_.connect(callback);
}

void Engine::OnPinch(float scale) {
  pinch_sig_(scale);
}

boost::signals2::connection Engine::AddMouseScrollCallback(const mouse_scroll_callback_t &callback) {
  return mouse_scroll_sig_.connect(callback);
}

void Engine::OnMouseScroll(int x_offset, int y_offset) {
  mouse_scroll_sig_(x_offset, y_offset);
}

boost::signals2::connection Engine::AddPassBeginCallback(const pass_callback_t &callback) {
  return pass_begin_sig_.connect(callback);
}

boost::signals2::connection Engine::AddPassEndCallback(const pass_callback_t &callback) {
  return pass_end_sig_.connect(callback);
}

boost::signals2::connection Engine::AddFrameMoveCallback(const frame_move_callback_t &callback) {
  return frame_move_sig_.connect(callback);
}

boost::signals2::connection Engine::AddSizeChangeCallback(const size_callback_t &callback) {
  return size_change_sig_.connect(callback);
}

void Engine::SetExtTextureLoaders(
  Engine::texture_loader_t file_loader,
  Engine::texture_mem_loader_t mem_loader) {
  ext_texture_loader_ = file_loader;
  ext_texture_mem_loader_ = mem_loader;
}

Engine::texture_loader_t Engine::ExtTextureLoader() {
  return ext_texture_loader_;
}

Engine::texture_mem_loader_t Engine::ExtTextureMemLoader() {
  return ext_texture_mem_loader_;
}

std::shared_ptr<SceneNode> Engine::Scene() const {
  return root_node_;
}


std::vector<RenderPass> &Engine::RenderPasses() {
  return render_passes_;
}

const std::vector<RenderPass> &Engine::RenderPasses() const {
  return render_passes_;
}

void Engine::DoRender(vertex_type::VertexArray &va, const std::string &material_name) {
   //resolve material
   std::shared_ptr<Material> material_ptr = bd_cast<Material>(
     resrc_mgr_.Find(material_name.c_str()));
   if (!override_material_.empty())
     material_ptr = bd_cast<Material>(resrc_mgr_.Find(override_material_));
   if (!material_ptr) //material not found, use default material
     material_ptr = bd_cast<Material>(resrc_mgr_.Find("mineola:material:fallback"));

   material_ptr->UploadToShader(current_effect_.second.get());
   va.Draw();
}

void Engine::CacheEffectFiles(const std::string &effect,
  const std::string &vs, const std::string &ps, const effect_defines_t *defines) {
  std::string tmp;
  effect_files_cache_[effect] =
    std::make_tuple(vs, tmp, ps, defines ? *defines : effect_defines_t());
}

void Engine::CacheEffectFiles(const std::string &effect,
  const std::string &vs, const std::string &gs, const std::string &ps,
  const effect_defines_t *defines) {
  effect_files_cache_[effect] =
    std::make_tuple(vs, gs, ps, defines ? *defines : effect_defines_t());
}

void Engine::ReloadEffectFiles() {
  for (const auto &kvp : effect_files_cache_) {
    if (std::get<1>(kvp.second).empty()) {
      ReloadEffectFromFileHelper(kvp.first.c_str(),
        std::get<0>(kvp.second).c_str(),
        std::get<2>(kvp.second).c_str(),
        &std::get<3>(kvp.second));
    }
  }

  ChangeEffect(current_effect_.first, true);
}

const std::weak_ptr<UniformBlock> &Engine::BuiltinUniformBlock() const {
  return builtin_uniform_block_;
}

bool Engine::TerminationSignaled() const {
  return terminate_signaled_;
}

void Engine::SignalTermination() {
  terminate_signaled_ = true;
}

#ifdef MINEOLA_LOG_TO_FILE
std::ostream &Engine::Log() {
  return log_;
}
#endif

} //namespace
