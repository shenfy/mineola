#include <mineola/RenderStateManager.h>
#include <mineola/glutility.h>
#include <glm/gtc/type_ptr.hpp>

namespace mineola {
RenderStateManager::RenderStateManager() {
}

RenderStateManager::~RenderStateManager() {
}

void RenderStateManager::ApplyStateCache(StateCache &state_cache, bool force) {
  state_cache.clear_state.Apply(*this, force);
  state_cache.front_face_state.Apply(*this, force);
  state_cache.cull_face_state.Apply(*this, force);
  state_cache.cull_enable_state.Apply(*this, force);
  state_cache.stencil_func_state.Apply(*this, force);
  state_cache.stencil_op_state.Apply(*this, force);
  state_cache.stencil_enable_state.Apply(*this, force);
  state_cache.depth_func_state.Apply(*this, force);
  state_cache.depth_test_state.Apply(*this, force);
  state_cache.depth_write_state.Apply(*this, force);
  state_cache.color_write_state.Apply(*this, force);
  state_cache.blend_func_state.Apply(*this, force);
  state_cache.blend_equation_state.Apply(*this, force);
  state_cache.blend_enable_state.Apply(*this, force);
}

void RenderStateManager::PushState() {
  states_stack_.push(current_state_);
}

void RenderStateManager::PopState() {
  auto &render_state = states_stack_.top();
  ApplyStateCache(render_state, false);
  states_stack_.pop();
}

/******************** ClearColor ********************/
void RenderStateManager::SetClearColor(const glm::vec4 &color, bool force) {
  if (force ||
    !glm::all(glm::equal(current_state_.clear_state.clear_color, color))) {
    glClearColor(color.x, color.y, color.z, color.w);
    current_state_.clear_state.clear_color = color;
  }
}

/******************** Cull ********************/
void RenderStateManager::SetFrontFace(
  render_state::FrontFaceOrder face_order, bool force) {
  if (force || current_state_.front_face_state.front_face_order != face_order) {
    glFrontFace(face_order);
    current_state_.front_face_state.front_face_order = face_order;
  }
}

void RenderStateManager::SetCullFace(
  render_state::CullFace face, bool force) {
  if (force || current_state_.cull_face_state.cull_face != face) {
    glCullFace(face);
    current_state_.cull_face_state.cull_face = face;
  }
}

void RenderStateManager::SetCullEnabled(bool enabled, bool force) {
  if (force || enabled != current_state_.cull_enable_state.enabled) {
    if (enabled) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }
    current_state_.cull_enable_state.enabled = enabled;
  }
}

/******************** Stencil test ********************/
void RenderStateManager::SetStencilFunc(
  render_state::CmpFunc func, int8_t ref, uint8_t mask, bool force) {
  if (force ||
    !(current_state_.stencil_func_state.stencil_func == func &&
      current_state_.stencil_func_state.ref == ref &&
      current_state_.stencil_func_state.mask == mask)) {
    glStencilFunc(func, ref, mask);
    current_state_.stencil_func_state.stencil_func = func;
    current_state_.stencil_func_state.ref = ref;
    current_state_.stencil_func_state.mask = mask;
  }
}

void RenderStateManager::SetStencilOp(
  render_state::StencilOp sfail,
  render_state::StencilOp dpfail,
  render_state::StencilOp dppass, bool force) {
  if (force ||
    !(sfail != current_state_.stencil_op_state.sfail &&
    dpfail != current_state_.stencil_op_state.dpfail &&
    dppass != current_state_.stencil_op_state.dppass)) {
    glStencilOp(sfail, dpfail, dppass);
    current_state_.stencil_op_state.sfail = sfail;
    current_state_.stencil_op_state.dpfail = dpfail;
    current_state_.stencil_op_state.dppass = dppass;
  }
}

void RenderStateManager::SetStencilTestEnabled(bool enabled, bool force) {
  if (force || enabled != current_state_.stencil_enable_state.enabled) {
    if (enabled) {
      glEnable(GL_STENCIL_TEST);
    } else {
      glDisable(GL_STENCIL_TEST);
    }
    current_state_.stencil_enable_state.enabled = enabled;
  }
}

/******************** Depth Test ********************/
void RenderStateManager::SetDepthFunc(
  render_state::CmpFunc func, bool force) {
  if (force || func != current_state_.depth_func_state.depth_func) {
    glDepthFunc(func);
    current_state_.depth_func_state.depth_func = func;
  }
}

void RenderStateManager::SetDepthTestEnabled(bool enabled, bool force) {
  if (force || enabled != current_state_.depth_test_state.enabled) {
    if (enabled) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
    current_state_.depth_test_state.enabled = enabled;
  }
}

void RenderStateManager::SetDepthWriteEnabled(bool enabled, bool force) {
  if (force || enabled != current_state_.depth_write_state.enabled) {
    glDepthMask(enabled);
    current_state_.depth_write_state.enabled = enabled;
  }
}

/******************** Color ********************/
void RenderStateManager::SetColorWrite(glm::bvec4 mask, bool force) {
  if (force ||
    !glm::all(glm::equal(current_state_.color_write_state.mask, mask))) {
    glColorMask(mask.x, mask.y, mask.z, mask.w);
    current_state_.color_write_state.mask = mask;
  }
}
/******************** Blend ********************/
void RenderStateManager::SetBlendFactor(
  render_state::BlendFactor src_factor,
  render_state::BlendFactor dst_factor, bool force) {
  if (force || src_factor != current_state_.blend_func_state.src_blend_factor ||
    dst_factor != current_state_.blend_func_state.dst_blend_factor) {
    glBlendFunc(src_factor, dst_factor);
    current_state_.blend_func_state.src_blend_factor = src_factor;
    current_state_.blend_func_state.dst_blend_factor = dst_factor;
  }
}

void RenderStateManager::SetBlendEquation(
  render_state::BlendEquation equation, bool force) {
  if (force || equation != current_state_.blend_equation_state.blend_equation) {
    glBlendEquation(equation);
    current_state_.blend_equation_state.blend_equation = equation;
  }
}

void RenderStateManager::SetBlendEnabled(bool enabled, bool force) {
  if (force || enabled != current_state_.blend_enable_state.enabled) {
    if (enabled) {
      glEnable(GL_BLEND);
    } else {
      glDisable(GL_BLEND);
    }
    current_state_.blend_enable_state.enabled = enabled;
  }
}

void RenderStateManager::ApplyCurrentState() {
  ApplyStateCache(current_state_, true);
}
}
