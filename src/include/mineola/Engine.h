#ifndef MINEOLA_RENDERENGINE_H
#define MINEOLA_RENDERENGINE_H

#ifdef MINEOLA_LOG_TO_FILE
#include <fstream>
#endif
#include "Camera.h"
#include "VertexType.h"
#include "RenderPass.h"
#include "BasisObj.h"
#include "Entity.h"
#include <boost/signals2.hpp>
#include "ManagerBase.h"
#include "ResourceManager.h"
#include "RenderStateManager.h"

namespace mineola {

namespace imgpp {
class Img;
}


//forward declaration
class GLEffect;
class Timer;
class Framebuffer;
class SceneNode;
class UniformBlock;

class Engine {
public:
  ~Engine();
  static Engine &Instance();

  ResourceManager &ResrcMgr();
  RenderStateManager &RenderStateMgr();
  ManagerBase<Entity> &EntityMgr();
  ManagerBase<Camera> &CameraMgr();

  void ChangeEffect(const std::string &name, bool force);
  std::shared_ptr<GLEffect> &CurrentEffect();

  void ChangeCamera(const std::string &name, bool force);
  std::shared_ptr<Camera> &CurrentCamera();

  void SetFramebuffer(const char *name, bool force, uint32_t viewport = 0); // defaults to the first viewport
  std::shared_ptr<Framebuffer> &CurrentFramebuffer();
  void SetViewport(uint32_t id); // viewport id within a framebuffer

  std::shared_ptr<Framebuffer> GetScrFramebuffer();

  void DoRender(vertex_type::VertexArray &va, const std::string &material_name);

    // manage render passes
  std::vector<RenderPass> &RenderPasses();
  const std::vector<RenderPass> &RenderPasses() const;

  void SetDefaultFramebuffer(uint32_t fbo, uint32_t width, uint32_t height);

  // life cycle
  void Init();
  void Start();
  void RenderGUI();  // not invoked unless using GUI
  void FrameMove();
  void Render();
  void OnSize(uint32_t width, uint32_t height);
  void Release();

  //signals management
  enum : uint8_t {BUTTON_DOWN = 0, BUTTON_UP};
  enum : uint8_t {MOUSE_LBUTTON = 0, MOUSE_RBUTTON = 1, MOUSE_MBUTTON = 2};

  using keyboard_signal_t = boost::signals2::signal<void (uint32_t, uint8_t)>;
  using keyboard_callback_t = keyboard_signal_t::slot_type;
  boost::signals2::connection AddKeyboardCallback(const keyboard_callback_t &callback);
  void OnKey(uint32_t key, uint8_t action);

  using mouse_btn_signal_t = boost::signals2::signal<void (uint8_t, uint8_t, int, int)>;
  using mouse_btn_callback_t = mouse_btn_signal_t::slot_type;
  boost::signals2::connection AddMouseButtonCallback(const mouse_btn_callback_t &callback);
  void OnMouseButton(uint8_t button, uint8_t action, int x, int y);

  using mouse_move_signal_t = boost::signals2::signal<void (int, int)>;
  using mouse_move_callback_t = mouse_move_signal_t::slot_type;
  boost::signals2::connection AddMouseMoveCallback(const mouse_move_callback_t &callback);
  void OnMouseMove(int x, int y);

  using mouse_scroll_signal_t = boost::signals2::signal<void (int, int)>;
  using mouse_scroll_callback_t = mouse_scroll_signal_t::slot_type;
  boost::signals2::connection AddMouseScrollCallback(const mouse_scroll_callback_t &callback);
  void OnMouseScroll(int x_offset, int y_offset);

  using pinch_signal_t = boost::signals2::signal<void (float)>;
  using pinch_callback_t = pinch_signal_t::slot_type;
  boost::signals2::connection AddPinchCallback(const pinch_callback_t &callback);
  void OnPinch(float scale);

  using pass_signal_t = boost::signals2::signal<void (uint32_t)>;
  using pass_callback_t = pass_signal_t::slot_type;
  boost::signals2::connection AddPassBeginCallback(const pass_callback_t &callback);
  boost::signals2::connection AddPassEndCallback(const pass_callback_t &callback);

  using frame_move_signal_t = boost::signals2::signal<void (double, double)>;
  using frame_move_callback_t = frame_move_signal_t::slot_type;
  boost::signals2::connection AddFrameMoveCallback(const frame_move_callback_t &callback);

  using size_signal_t = boost::signals2::signal<void (uint32_t, uint32_t)>;
  using size_callback_t = size_signal_t::slot_type;
  boost::signals2::connection AddSizeChangeCallback(const size_callback_t &callback);

  // time
  double LastFrameTime();
  double Now();

  // scene graph
  std::shared_ptr<SceneNode> Scene() const;

  // effects
  using effect_defines_t = std::vector<std::pair<std::string, std::string>>;
  using effect_files_cache_t = std::unordered_map<
    std::string,
    std::tuple<std::string, std::string, std::string, effect_defines_t>>;
  void CacheEffectFiles(const std::string &effect, const std::string &vs, const std::string &ps,
    const effect_defines_t *defines);
  void CacheEffectFiles(const std::string &effect,
    const std::string &vs, const std::string &gs, const std::string &ps,
    const effect_defines_t *defines);
  void ReloadEffectFiles();

  // uniform block
  const std::weak_ptr<UniformBlock> &BuiltinUniformBlock() const;

  // external texture loaders
  using texture_loader_t = std::add_pointer<bool(const char *, bool, imgpp::Img &)>::type;
  using texture_mem_loader_t =
    std::add_pointer<bool(const char *, uint32_t, bool, imgpp::Img &)>::type;
  void SetExtTextureLoaders(texture_loader_t file_loader, texture_mem_loader_t mem_loader);
  texture_loader_t ExtTextureLoader();
  texture_mem_loader_t ExtTextureMemLoader();

  // termination
  bool TerminationSignaled() const;
  void SignalTermination();

  #ifdef MINEOLA_LOG_TO_FILE
  // log
  std::ostream &Log();
  #endif

private:
  Engine();

  // managers
  ResourceManager resrc_mgr_;
  RenderStateManager render_state_mgr_;
  ManagerBase<Entity> entity_mgr_;
  ManagerBase<Camera> camera_mgr_;
  std::shared_ptr<SceneNode> root_node_;

  // current state cache
  std::pair<std::string, std::shared_ptr<GLEffect> > current_effect_;
  std::pair<std::string, std::shared_ptr<Camera> > current_camera_;
  std::pair<std::string, std::shared_ptr<Framebuffer> > current_framebuffer_;
  uint32_t current_viewport_;
  std::weak_ptr<Framebuffer> scr_framebuffer_;

  // passes
  std::vector<RenderPass> render_passes_;

  // signals
  keyboard_signal_t keyboard_sig_;
  mouse_btn_signal_t mouse_button_sig_;
  mouse_move_signal_t mouse_move_sig_;
  mouse_scroll_signal_t mouse_scroll_sig_;
  pass_signal_t pass_begin_sig_;
  pass_signal_t pass_end_sig_;
  frame_move_signal_t frame_move_sig_;
  size_signal_t size_change_sig_;
  pinch_signal_t pinch_sig_;

  std::shared_ptr<Timer> timer_;
  double time_, frame_time_;

  bool override_effect_;
  bool override_camera_;
  bool override_render_target_;
  std::string override_material_;

  effect_files_cache_t effect_files_cache_;

  texture_loader_t ext_texture_loader_;
  texture_mem_loader_t ext_texture_mem_loader_;

  std::weak_ptr<UniformBlock> builtin_uniform_block_;

  bool terminate_signaled_;

  #ifdef MINEOLA_LOG_TO_FILE
  std::ofstream log_;
  #endif
};

}

#endif
