#include "prefix.h"
#include <mineola/Material.h>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/GLEffect.h>
#include <mineola/Engine.h>
#include <mineola/Texture.h>
#include <mineola/glutility.h>
#include <mineola/ReservedTextureUnits.h>

namespace {
    const mineola::TextureTransform kDefaultTransform;
}

namespace mineola {

void TextureTransform::UploadToShader(const char *tex_name, GLEffect *effect) const {
  std::string var_name = tex_name;
  effect->UploadVariable((var_name + "_ts").c_str(), glm::value_ptr(offset_scale));
  effect->UploadVariable((var_name + "_rot").c_str(), &rotation);
}

void Material::UploadToShader(GLEffect *effect) {
  effect->UploadVariable("ambient", glm::value_ptr(ambient));
  effect->UploadVariable("diffuse", glm::value_ptr(diffuse));
  effect->UploadVariable("specular", glm::value_ptr(specular));
  effect->UploadVariable("emit", glm::value_ptr(emit));
  effect->UploadVariable("alpha", &alpha);
  effect->UploadVariable("specularity", &specularity);
  effect->UploadVariable("roughness", &roughness);

  // upload uniforms
  for (const auto &kvp : uniform_slots) {
    kvp.second->UploadToShader(kvp.first.c_str(), effect);
  }

  // upload textures
  int32_t tex_unit = kNumReservedTextureUnits;
  for (auto iter = texture_slots.begin(); iter != texture_slots.end(); ++iter) {
    static std::vector<int32_t> tex_units;
    if (iter->second.size() > tex_units.size()) {
     tex_units.resize(iter->second.size());
    }

    bool all_found = true;
    for (size_t i = 0; i < iter->second.size(); ++i) {
      auto texture = bd_cast<Texture>(
        Engine::Instance().ResrcMgr().Find(iter->second[i].c_str()));
      if (!texture) {
        all_found = false;
        break;
      }

      tex_units[i] = tex_unit;
      glActiveTexture(GL_TEXTURE0 + tex_unit);
      texture->Bind();
      ++tex_unit;
    }
    if (!all_found)
      continue;

    effect->UploadVariable(iter->first.c_str(), &tex_units[0]);
    
    if (auto trs_iter = texture_tforms.find(iter->first); trs_iter != texture_tforms.end()) {
      trs_iter->second.UploadToShader(iter->first.c_str(), effect);
    } else {
      kDefaultTransform.UploadToShader(iter->first.c_str(), effect);
    }
 }
}

} //namespace
