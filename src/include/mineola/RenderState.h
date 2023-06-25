#ifndef MINEOLA_RENDERSTATE_H
#define MINEOLA_RENDERSTATE_H

#include "GLMDefines.h"
#include <glm/glm.hpp>
#include "glutility.h"

namespace mineola {
class RenderStateManager;

namespace render_state {
// Counter-clockwise is front(default)
enum FrontFaceOrder {
  kFrontFaceOrderCCW = GL_CCW,
  kFrontFaceOrderCW = GL_CW
};

// Cull back faces(default)
enum CullFace {
  kCullFaceBack = GL_BACK,
  kCullFaceFront = GL_FRONT,
  kCullFaceFrontAndBack = GL_FRONT_AND_BACK
};

enum CmpFunc{
  kCmpFuncNever = GL_NEVER,  // Always fails
  kCmpFuncLess = GL_LESS,  // Passes if (ref & mask) < (stencil & mask)
  kCmpFuncLEqual = GL_LEQUAL,  // Passes if (ref & mask) <= (stencil & mask)
  kCmpFuncGreater = GL_GREATER,  // Passes if (ref & maks) > (stencil & mask)
  kCmpFuncGEqual = GL_GEQUAL,  // Passes if (ref & mask) >= (stencil & mask)
  kCmpFuncEqual = GL_EQUAL,  // Passes if (ref & mask) = (stencil & mask)
  kCmpFuncNotEqual = GL_NOTEQUAL,  // Passes if (ref & mask) != (stencil & mask)
  kCmpFuncAlways = GL_ALWAYS  // Always passes
};

enum StencilOp {
  kStencilKeep = GL_KEEP,  // Don't modify the current value (default)
  kStencilZero = GL_ZERO,  // Set it to zero
  kStencilReplace = GL_REPLACE,  // Replace with the masked fragment value
  kStencilIncr = GL_INCR,  // Increment the current value, saturating 1 if it would overflow
  kStencilIncrWrap = GL_INCR_WRAP,  // Increment the current value, wrapping if it would overflow
  kStencilDecr = GL_DECR,  // Decrement the current value, setting to zero if it would underflow
  kStencilDecrWrap = GL_DECR_WRAP,  // Decrement the current value, wrapping if it would underflow
  kStencilInvert = GL_INVERT  // Invert the current value
};

// Blend src are incoming values, dst are values already in frame buffer
enum BlendFactor {
  kBlendZero = GL_ZERO,
  kBlendOne = GL_ONE,
  kBlendSrcColor = GL_SRC_COLOR,
  kBlendOneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
  kBlendDstColor = GL_DST_COLOR,
  kBlendOneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
  kBlendSrcAlpha = GL_SRC_ALPHA,
  kBlendOneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
  kBlendDstAlpha = GL_DST_ALPHA,
  kBlendOneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
  kBlendConstantColor = GL_CONSTANT_COLOR,
  kBlendOneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
  kBlendConstantAlpha = GL_CONSTANT_ALPHA,
  kBlendOneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
  kBlendSrcAlphaSaturate = GL_SRC_ALPHA_SATURATE,
};

enum BlendEquation {
  kBlendEquationFuncAdd = GL_FUNC_ADD,
  kBlendEquationFuncSubtract = GL_FUNC_SUBTRACT,  // src - dst
  kBlendEquationFuncReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,  // dst - src
  kBlendEquationMin = GL_MIN,
  kBlendEquationMax = GL_MAX
};
}

struct RenderState {
public:
  virtual ~RenderState();
  virtual void Apply(RenderStateManager &render_state_mgr, bool force = false) const = 0;
};

struct ClearState : public RenderState {
  glm::vec4 clear_color;

  ClearState();
  ClearState(const glm::vec4 &clear_color);
  ClearState(float r, float g, float b, float a);
  ~ClearState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct FrontFaceState : public RenderState {
  render_state::FrontFaceOrder front_face_order;

  FrontFaceState();
  FrontFaceState(render_state::FrontFaceOrder order);
  ~FrontFaceState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct CullFaceState : public RenderState {
  render_state::CullFace cull_face;

  CullFaceState();
  CullFaceState(render_state::CullFace cull_face);
  ~CullFaceState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct CullEnableState : public RenderState {
  bool enabled;

  CullEnableState();
  CullEnableState(bool enabled);
  ~CullEnableState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct StencilFuncState : public RenderState {
  render_state::CmpFunc stencil_func;
  int8_t ref;
  uint8_t mask;
  StencilFuncState();
  StencilFuncState(render_state::CmpFunc stencil_func, int8_t ref, uint8_t mask);
  ~StencilFuncState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct StencilOpState : public RenderState {
  render_state::StencilOp sfail;
  render_state::StencilOp dpfail;
  render_state::StencilOp dppass;

  StencilOpState();
  StencilOpState(
    render_state::StencilOp sfail,
    render_state::StencilOp dpfail,
    render_state::StencilOp dppass);
  ~StencilOpState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct StencilEnableState : public RenderState {
  bool enabled;

  StencilEnableState();
  StencilEnableState(bool enable);
  ~StencilEnableState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct DepthFuncState: public RenderState {
  render_state::CmpFunc depth_func;

  DepthFuncState();
  DepthFuncState(render_state::CmpFunc depth_func);
  ~DepthFuncState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct DepthTestState : public RenderState {
  bool enabled;

  DepthTestState();
  DepthTestState(bool enable);
  ~DepthTestState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct DepthWriteState : public RenderState {
  bool enabled;

  DepthWriteState();
  DepthWriteState(bool enable);
  ~DepthWriteState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct ColorWriteState : public RenderState {
  glm::bvec4 mask;

  ColorWriteState();
  ColorWriteState(const glm::bvec4 &mask);
  ColorWriteState(bool r_mask, bool g_mask, bool b_mask, bool a_mask);
  ~ColorWriteState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct BlendFuncState: public RenderState {
  render_state::BlendFactor src_blend_factor;
  render_state::BlendFactor dst_blend_factor;

  BlendFuncState();
  BlendFuncState(
    render_state::BlendFactor src_blend_factor,
    render_state::BlendFactor dst_blend_factor);
  ~BlendFuncState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};


struct BlendEquationState : public RenderState {
  render_state::BlendEquation blend_equation;

  BlendEquationState();
  BlendEquationState(render_state::BlendEquation blend_equation);
  ~BlendEquationState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

struct BlendEnableState : public RenderState {
  bool enabled;

  BlendEnableState();
  BlendEnableState(bool enable);
  ~BlendEnableState() final;
  void Apply(RenderStateManager &render_state_mgr, bool force=false) const override;
};

}
#endif
