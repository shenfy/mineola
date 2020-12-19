#include "prefix.h"
#include <mineola/EnvLight.h>
#include <fstream>
#include <imgpp/imgpp.hpp>
#include <imgpp/compositeimg.hpp>
#include <imgpp/loaders.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/UniformBlock.h>
#include <mineola/Engine.h>
#include <mineola/Rbt.h>
#include <mineola/TextureHelper.h>
#include <mineola/ImgppTextureSrc.h>
#include <mineola/ReservedTextureUnits.h>

namespace mineola {

EnvLight::EnvLight(size_t idx)
  : Light(idx) {
}

EnvLight::~EnvLight() = default;

void EnvLight::UpdateUniforms(UniformBlock *ub) {
  auto mat_name = "_env_light_mat_" + std::to_string(idx_);
  ub->UpdateVariable(mat_name.c_str(), glm::value_ptr(mat_));

  auto sh3_name = "_env_light_sh3_" + std::to_string(idx_) + "[0]";
  ub->UpdateVariable(sh3_name.c_str(), sh3_.coeffs.data());
}

void EnvLight::UpdateLightTransform(const math::Rbt &rbt) {
  mat_ = math::Rbt::Inv(rbt).ToMatrix();
}

bool EnvLight::LoadFromFile(const char *fn) {
  // load light probe file
  imgpp::CompositeImg light_probe;
  std::unordered_map<std::string, std::string> custom_data;
  if (!imgpp::LoadKTX(fn, light_probe, custom_data, false)) {
    MLOG("Failed to load env light probe: %s\n", fn);
    return false;
  }

  // create light probe texture
  using namespace texture_helper;
  if (!CreateLightProbeTexture(CreateTextureSrc(light_probe))) {
    return false;
  }

  // record sh coeffcients
  auto it = custom_data.find("sh");
  if (it == custom_data.end()) {
    MLOG("Failed to find SH coeffcients in env light probe\n");
    return false;
  }

  if (it->second.size() != sh3_.coeffs.size() * sizeof(glm::vec3)) {
    MLOG("Invalid env light probe SH coeffcients!\n");
  }

  auto coeffs = reinterpret_cast<const glm::vec3*>(it->second.data());
  std::transform(coeffs, coeffs + sh3_.coeffs.size(), sh3_.coeffs.begin(),
    [](auto &v3) {
      return glm::vec4(v3, 0.0f);
    }
  );

  return true;
}

bool EnvLight::CreateLightProbeTexture(std::shared_ptr<ImgppTextureSrc> tex_src) {
  // create texture desc and check validity
  TextureDesc desc;
  if (!texture_helper::CreateTextureDesc(tex_src, false, true,
    TextureDesc::kLinearMipmapLinear, TextureDesc::kLinear,
    TextureDesc::kRepeat, TextureDesc::kClampToEdge, desc)) {
    MLOG("Failed to create texture desc from env light probe\n");
    return false;
  }
  if (desc.type != GL_TEXTURE_2D || desc.depth != 1 || desc.array_size != 1) {
    MLOG("Env light probe texture must be mipmapped 2D\n");
    return false;
  }

  // create light probe texture
  light_probe_ = std::make_shared<Texture2D>();
  if (!light_probe_->Create(desc)) {
    MLOG("Failed to create env light probe texture\n");
    return false;
  }

  // bind to texture unit
  if (idx_ == 0) {
    glActiveTexture(GL_TEXTURE0 + kEnvLightProbe0TextureUnit);
    light_probe_->Bind();
    glActiveTexture(GL_TEXTURE0 + kNumReservedTextureUnits);
  }

  return true;
}

}  // end namespace
