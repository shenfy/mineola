#include "prefix.h"
#include "../include/RenderPass.h"

namespace mineola {
  RenderPass CreateDepthRenderPass(const char *render_target) {
    RenderPass result;
    result.layer_mask = RenderPass::RENDER_LAYER_0;
    result.sfx = RenderPass::SFX_PASS_DEPTH;
    result.override_effect = "mineola:effect:depthmap";
    if (render_target != nullptr) result.override_render_target = render_target;
    return result;
  }

  RenderPass CreateShadowmapPass() {
    RenderPass result;
    result.layer_mask = RenderPass::RENDER_LAYER_CAST_SHADOW;
    result.sfx = RenderPass::SFX_PASS_SHADOWMAP;
    result.override_effect = "mineola:effect:shadowmap";
    result.override_render_target = "mineola:rt:shadowmap";
    return result;
  }
} //end namespace