#ifndef MINEOLA_RENDERSTATEFACTORY_H
#define MINEOLA_RENDERSTATEFACTORY_H

#include <memory>
#include <string>
#include "RenderState.h"

namespace mineola {
struct RenderState;

class RenderStateFactory {
public:
  static std::unique_ptr<RenderState> CreateRenderState(
    const std::string &state_name, const std::string &params_str);
};
}
#endif
