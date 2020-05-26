#include <mineola/RenderState.h>
#include <mineola/RenderStateManager.h>

namespace mineola {

RenderState::~RenderState() {
}

ClearState::ClearState() :
  clear_color(0.f, 0.f, 0.f, 0.f) {
}

ClearState::ClearState(const glm::vec4 &color) :
  clear_color(color) {
}

ClearState::ClearState(float r, float g, float b, float a) :
  clear_color(r, g, b, a) {
}

ClearState::~ClearState() = default;

void ClearState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetClearColor(clear_color, force);
}

FrontFaceState::FrontFaceState() :
  front_face_order(render_state::kFrontFaceOrderCCW) {
}

FrontFaceState::FrontFaceState(
  render_state::FrontFaceOrder order) :
  front_face_order(order) {
}

FrontFaceState::~FrontFaceState() = default;

void FrontFaceState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetFrontFace(front_face_order, force);
}


CullFaceState::CullFaceState() :
  cull_face(render_state::kCullFaceBack) {
}

CullFaceState::CullFaceState(
  render_state::CullFace face) :
  cull_face(face) {
}

CullFaceState::~CullFaceState() = default;

void CullFaceState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetCullFace(cull_face, force);
}


CullEnableState::CullEnableState() :
  enabled(false) {
}

CullEnableState::CullEnableState(bool enabled) :
  enabled(enabled) {
}

CullEnableState::~CullEnableState() = default;

void CullEnableState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetCullEnabled(enabled, force);
}


StencilFuncState::StencilFuncState() :
  stencil_func(render_state::kCmpFuncAlways),
  ref(1), mask(0xff) {
}

StencilFuncState::StencilFuncState(
  render_state::CmpFunc func, int8_t ref, uint8_t mask) :
  stencil_func(func), ref(ref), mask(mask) {
}

StencilFuncState::~StencilFuncState() = default;

void StencilFuncState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetStencilFunc(stencil_func, ref, mask, force);
}


StencilOpState::StencilOpState() :
  sfail(render_state::kStencilKeep),
  dpfail(render_state::kStencilKeep),
  dppass(render_state::kStencilKeep) {
}

StencilOpState::StencilOpState(
  render_state::StencilOp sfail,
  render_state::StencilOp dpfail,
  render_state::StencilOp dppass) :
  sfail(sfail), dpfail(dpfail), dppass(dppass) {
}

StencilOpState::~StencilOpState() = default;

void StencilOpState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetStencilOp(sfail, dpfail, dppass, force);
}


StencilEnableState::StencilEnableState() :
  enabled(false) {
}

StencilEnableState::StencilEnableState(bool enabled) :
  enabled(enabled) {
}

StencilEnableState::~StencilEnableState() = default;

void StencilEnableState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetStencilTestEnabled(enabled, force);
}


DepthFuncState::DepthFuncState() :
  depth_func(render_state::kCmpFuncLess) {
}

DepthFuncState::DepthFuncState(
  render_state::CmpFunc func) :
  depth_func(func) {
}

DepthFuncState::~DepthFuncState() = default;

void DepthFuncState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetDepthFunc(depth_func, force);
}


DepthTestState::DepthTestState() :
  enabled(false) {
}

DepthTestState::DepthTestState(bool enabled) :
  enabled(enabled) {
}

DepthTestState::~DepthTestState() = default;

void DepthTestState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetDepthTestEnabled(enabled, force);
}

DepthWriteState::DepthWriteState() :
  enabled(true) {
}

DepthWriteState::DepthWriteState(bool enabled) :
  enabled(enabled) {
}

DepthWriteState::~DepthWriteState() = default;

void DepthWriteState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetDepthWriteEnabled(enabled, force);
}

ColorWriteState::ColorWriteState() :
  mask(true, true, true, true) {
}

ColorWriteState::ColorWriteState(const glm::bvec4 &mask) :
  mask(mask) {
}

ColorWriteState::ColorWriteState(
  bool r_mask, bool g_mask, bool b_mask, bool a_mask) :
  mask(r_mask, g_mask, b_mask, a_mask) {
}

ColorWriteState::~ColorWriteState() = default;

void ColorWriteState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetColorWrite(mask, force);
}

BlendFuncState::BlendFuncState() :
  src_blend_factor(render_state::kBlendOne),
  dst_blend_factor(render_state::kBlendZero) {
}

BlendFuncState::BlendFuncState(
  render_state::BlendFactor src_factor,
  render_state::BlendFactor dst_factor) :
  src_blend_factor(src_factor),
  dst_blend_factor(dst_factor) {
}

BlendFuncState::~BlendFuncState() = default;

void BlendFuncState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetBlendFactor(
    src_blend_factor, dst_blend_factor, force);
}


BlendEquationState::BlendEquationState() :
  blend_equation(render_state::kBlendEquationFuncAdd) {
}

BlendEquationState::BlendEquationState(
  render_state::BlendEquation equation) :
  blend_equation(equation) {
}

BlendEquationState::~BlendEquationState() = default;

void BlendEquationState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetBlendEquation(blend_equation, force);
}

BlendEnableState::BlendEnableState() :
  enabled(false) {
}

BlendEnableState::BlendEnableState(bool enabled) :
  enabled(enabled) {
}

BlendEnableState::~BlendEnableState() = default;

void BlendEnableState::Apply(
  RenderStateManager &render_state_mgr, bool force) const {
  render_state_mgr.SetBlendEnabled(enabled, force);
}
}
