#ifndef MINEOLA_RENDERSTATEMANAGER_H
#define MINEOLA_RENDERSTATEMANAGER_H

#include <unordered_map>
#include <stack>
#include <glm/glm.hpp>
#include "RenderState.h"

namespace mineola {

class RenderStateManager {
public:

  RenderStateManager();
  ~RenderStateManager();

  void ApplyCurrentState();
  void PushState();
  void PopState();
  
  void SetClearColor(const glm::vec4 &clear_color, bool force = false);

  void SetFrontFace(
    render_state::FrontFaceOrder face_order, bool force = false);
  void SetCullFace(
    render_state::CullFace face, bool force = false);
  void SetCullEnabled(bool enabled, bool force = false);

  void SetStencilFunc(
    render_state::CmpFunc func, int8_t ref, uint8_t mask, bool force = false);
  /* sfail specifies stencil test failed
   * dpfail specifies stencil test passes but depth test failed
   * dppass specifies stencil test and depth test both pass
  */
  
  void SetStencilOp(
    render_state::StencilOp sfail,
    render_state::StencilOp dpfail,
    render_state::StencilOp dppass,
    bool force = false);
  void SetStencilTestEnabled(bool enabled, bool force = false);

  void SetDepthFunc(
    render_state::CmpFunc func, bool force = false);
  void SetDepthTestEnabled(bool enabled, bool force = false);
  void SetDepthWriteEnabled(bool enabled, bool force = false);

  void SetColorWrite(glm::bvec4 mask, bool force = false);

  void SetBlendFactor(
    render_state::BlendFactor src_factor,
    render_state::BlendFactor dst_factor, bool force = false);
  void SetBlendEquation(
    render_state::BlendEquation equation, bool force = false);
  void SetBlendEnabled(bool enabled, bool force = false);

  int32_t GetCullMode() const;
  int32_t GetStencilMode() const;
  int32_t GetDepthTestMode() const;
  int32_t GetBlendMode() const;

protected:
  struct StateCache {
    ClearState clear_state;
    FrontFaceState front_face_state;
    CullFaceState cull_face_state;
    CullEnableState cull_enable_state;
    StencilFuncState stencil_func_state;
    StencilOpState stencil_op_state;
    StencilEnableState stencil_enable_state;
    DepthFuncState depth_func_state;
    DepthTestState depth_test_state;
    DepthWriteState depth_write_state;
    ColorWriteState color_write_state;
    BlendFuncState blend_func_state;
    BlendEquationState blend_equation_state;
    BlendEnableState blend_enable_state;
  };
  void ApplyStateCache(StateCache &state_cache, bool force = false);
  StateCache current_state_;
  std::stack<StateCache> states_stack_;
};
}

#endif  // namespace