#include "../include/RenderStateFactory.h"
#include "../include/RenderState.h"
#include "../include/GLMHelper.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace mineola {

using namespace render_state;

namespace {
enum {
  kClearState = 0,
  kFrontFaceState,
  kCullFaceState,
  kCullEnableState,
  kStencilFuncState,
  kStencilOpState,
  kStencilEnableState,
  kDepthFuncState,
  kDepthTestState,
  kDepthWriteState,
  kDepthClampState,
  kColorWriteState,
  kBlendFuncState,
  kBlendEquationState,
  kBlendEnableState
};

int StateNameToEnum(const std::string &state_name) {
  static std::unordered_map<std::string, int> states_map = {
    {"clearcolor" ,kClearState},
    {"frontface" ,kFrontFaceState},
    {"cullface" ,kCullFaceState},
    {"cullenable" ,kCullEnableState},
    {"stencilfunc" ,kStencilFuncState},
    {"stencilop" ,kStencilOpState},
    {"stencilenable" ,kStencilEnableState},
    {"depthfunc" ,kDepthFuncState},
    {"depthtest" ,kDepthTestState},
    {"depthwrite" ,kDepthWriteState},
    {"depthclamp" ,kDepthClampState},
    {"colorwrite", kColorWriteState},
    {"blendfunc" ,kBlendFuncState},
    {"blendequation" ,kBlendEquationState},
    {"blendenable" ,kBlendEnableState}
  };
  auto it = states_map.find(boost::algorithm::trim_copy(state_name));
  if (it == states_map.end()) {
    MLOG("%s not supported yet!\n", state_name.c_str());
    return -1;
  }
  return it->second;
}

int StateParamToEnum(const std::string &state_param) {
  static std::unordered_map<std::string, int> state_params_map = {
    {"on", 1},
    {"off", 0},
    {"ccw", kFrontFaceOrderCCW},
    {"cw", kFrontFaceOrderCW},
    {"back", kCullFaceBack},
    {"front", kCullFaceFront},
    {"frontback", kCullFaceFrontAndBack},

    {"never", kCmpFuncNever},
    {"less", kCmpFuncLess},
    {"lequal", kCmpFuncLEqual},
    {"greater", kCmpFuncGreater},
    {"gequal", kCmpFuncGEqual},
    {"equal", kCmpFuncEqual},
    {"notequal", kCmpFuncNotEqual},
    {"always", kCmpFuncAlways},

    {"sop_keep", kStencilKeep},
    {"sop_zero", kStencilZero},
    {"sop_replace", kStencilReplace},
    {"sop_incr", kStencilIncr},
    {"sop_incrwarp", kStencilIncrWrap},
    {"sop_decr", kStencilDecr},
    {"sop_decrwarp", kStencilDecrWrap},
    {"sop_inv", kStencilInvert},

    {"blend_zero", kBlendZero},
    {"blend_one", kBlendOne},
    {"src_color", kBlendSrcColor},
    {"one_minus_src_color", kBlendOneMinusSrcColor},
    {"dst_color", kBlendDstColor},
    {"one_minus_dst_color", kBlendOneMinusDstColor},
    {"src_alpha", kBlendSrcAlpha},
    {"one_minus_src_alpha", kBlendOneMinusSrcAlpha},
    {"dst_alpha", kBlendDstAlpha},
    {"one_minus_dst_alpha", kBlendOneMinusDstAlpha},
    {"const_color", kBlendConstantColor},
    {"one_minus_const_color", kBlendOneMinusConstantColor},
    {"const_alpha", kBlendConstantAlpha},
    {"one_minus_const_alpha", kBlendOneMinusConstantAlpha},
    {"src_alpha_saturate", kBlendSrcAlphaSaturate},

    {"blend_eqn_add", kBlendEquationFuncAdd},
    {"blend_eqn_sub", kBlendEquationFuncSubtract},
    {"blend_eqn_reverse_sub", kBlendEquationFuncReverseSubtract},
    {"blend_eqn_min", kBlendEquationMin},
    {"blend_eqn_max", kBlendEquationMax}
  };
  auto it = state_params_map.find(boost::algorithm::trim_copy(state_param));
  if (it == state_params_map.end()) {
    MLOG("%s not supported yet!\n", state_param.c_str());
    return -1;
  }
  return it->second;
}
}

std::unique_ptr<RenderState> RenderStateFactory::CreateRenderState(
    const std::string &state_name, const std::string &params_str) {
  int state = StateNameToEnum(state_name);
  switch(state) {
    case kClearState:
      return std::make_unique<ClearState>(ParseVec4(params_str));
    case kFrontFaceState: {
      int front_face = StateParamToEnum(params_str);
      if (front_face != -1)
        return std::make_unique<FrontFaceState>(FrontFaceOrder(front_face));
      return std::unique_ptr<RenderState>();
    }
    case kCullFaceState: {
      int cull_face = StateParamToEnum(params_str);
      if (cull_face != -1)
        return std::make_unique<CullFaceState>(CullFace(cull_face));
      return std::unique_ptr<RenderState>();
    }
    case kCullEnableState: {
      int enabled = StateParamToEnum(params_str);
      if (enabled != -1)
        return std::make_unique<CullEnableState>(enabled);
      return std::unique_ptr<RenderState>();
    }
    case kStencilFuncState: {
      std::vector<std::string> str_vec;
      boost::algorithm::split(str_vec, params_str, boost::algorithm::is_any_of(","));
      if (str_vec.size() != 3)
        return std::unique_ptr<RenderState>();
      int func = StateParamToEnum(str_vec[0]);
      if (func == -1)
        return std::unique_ptr<RenderState>();
      int8_t ref = (uint8_t)std::stoi(str_vec[1]);
      uint8_t mask = (uint8_t)std::stoi(str_vec[2], 0, 16);
      return std::make_unique<StencilFuncState>(CmpFunc(func), ref, mask);
    }
    case kStencilOpState: {
      std::vector<std::string> str_vec;
      boost::algorithm::split(str_vec, params_str, boost::algorithm::is_any_of(","));
      if (str_vec.size() != 3)
        return std::unique_ptr<RenderState>();
      int sfail = StateParamToEnum(str_vec[0]);
      int dpfail = StateParamToEnum(str_vec[1]);
      int dppass = StateParamToEnum(str_vec[2]);
      if (sfail != -1 && dpfail != -1 && dppass != -1)
        return std::make_unique<StencilOpState>(
          StencilOp(sfail), StencilOp(dpfail), StencilOp(dppass));
      return std::unique_ptr<RenderState>();
    }
    case kStencilEnableState: {
      int enabled = StateParamToEnum(params_str);
      if (enabled != -1)
        return std::make_unique<StencilEnableState>(enabled);
      return std::unique_ptr<RenderState>();
    }
    case kDepthFuncState: {
      int func = StateParamToEnum(params_str);
      if ( func != -1)
        return std::make_unique<DepthFuncState>(CmpFunc(func));
      return std::unique_ptr<RenderState>();
    }
    case kDepthTestState: {
      int enabled = StateParamToEnum(params_str);
      if (enabled != -1)
        return std::make_unique<DepthTestState>(enabled);
      break;
    }
    case kDepthWriteState: {
      int enabled = StateParamToEnum(params_str);
      if (enabled != -1)
        return std::make_unique<DepthWriteState>(enabled);
      break;
    }
    case kColorWriteState: {
      std::vector<std::string> str_vec;
      boost::algorithm::split(str_vec, params_str, boost::algorithm::is_any_of(","));
      if (str_vec.size() != 4)
        return std::unique_ptr<RenderState>();
      int r_write = StateParamToEnum(str_vec[0]);
      int g_write = StateParamToEnum(str_vec[1]);
      int b_write = StateParamToEnum(str_vec[2]);
      int a_write = StateParamToEnum(str_vec[3]);
      if (r_write != -1 && g_write != -1 && b_write != -1 && a_write != -1)
        return std::make_unique<ColorWriteState>(
          r_write, g_write, b_write, a_write);
      return std::unique_ptr<RenderState>();
    }
    case kBlendFuncState: {
      std::vector<std::string> str_vec;
      boost::algorithm::split(str_vec, params_str, boost::algorithm::is_any_of(","));
      if (str_vec.size() != 2)
        return std::unique_ptr<RenderState>();
      int src_factor = StateParamToEnum(str_vec[0]);
      int dst_factor = StateParamToEnum(str_vec[1]);
      if (src_factor != -1 && dst_factor != -1)
        return std::make_unique<BlendFuncState>(
          BlendFactor(src_factor), BlendFactor(dst_factor));
      return std::unique_ptr<RenderState>();
    }
    case kBlendEquationState: {
      int blend_eqn = StateParamToEnum(params_str);
      if (blend_eqn != -1)
        return std::make_unique<BlendEquationState>(BlendEquation(blend_eqn));
      return std::unique_ptr<RenderState>();
    }
    case kBlendEnableState: {
      int enabled = StateParamToEnum(params_str);
      if (enabled != -1)
        return std::make_unique<BlendEnableState>(enabled);
      return std::unique_ptr<RenderState>();
    }
    default:
      return std::unique_ptr<RenderState>();
  }
  return nullptr;
}
}
