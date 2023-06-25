#ifndef MINEOLA_RENDERPASS_H
#define MINEOLA_RENDERPASS_H

#include <string>
#include "GLMDefines.h"
#include <glm/glm.hpp>

namespace mineola {

struct RenderPass {
  enum { RENDER_LAYER_NONE = 0,
    RENDER_LAYER_0 = 1 << 0,
    RENDER_LAYER_1 = 1 << 1,
    RENDER_LAYER_2 = 1 << 2,
    RENDER_LAYER_3 = 1 << 3,
    RENDER_LAYER_4 = 1 << 4,
    RENDER_LAYER_5 = 1 << 5,
    RENDER_LAYER_6 = 1 << 6,
    RENDER_LAYER_7 = 1 << 7,
    RENDER_LAYER_8 = 1 << 8,
    RENDER_LAYER_9 = 1 << 9,
    RENDER_LAYER_10 = 1 << 10,
    RENDER_LAYER_11 = 1 << 11,
    RENDER_LAYER_12 = 1 << 12,
    RENDER_LAYER_13 = 1 << 13,
    RENDER_LAYER_14 = 1 << 14,
    RENDER_LAYER_15 = 1 << 15,
    RENDER_LAYER_CAST_SHADOW = 1 << 16,
    RENDER_LAYER_ALL = 0xFFFF};

  enum { SFX_PASS_NONE = 0, SFX_PASS_DEPTH, SFX_PASS_SHADOWMAP, SFX_PASS_HDR };

  enum { CLEAR_NONE = 0, CLEAR_DEPTH = 0x1, CLEAR_COLOR = 0x2, CLEAR_ALL = 0x1 | 0x2 };

  int layer_mask {RENDER_LAYER_ALL};
  int sfx {SFX_PASS_NONE};
  int clear_flag {CLEAR_ALL};
  std::string override_effect;
  std::string override_render_target;
  std::string override_camera;
  std::string override_material;
};

RenderPass CreateShadowmapPass();

} //end namespace

#endif
