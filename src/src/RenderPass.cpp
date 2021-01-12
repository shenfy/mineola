#include "prefix.h"
#include <mineola/RenderPass.h>

namespace mineola {

RenderPass CreateShadowmapPass() {
  RenderPass result;
  result.sfx = RenderPass::SFX_PASS_SHADOWMAP;
  result.override_render_target = "mineola:rt:shadowmap";
  return result;
}

} //end namespace
