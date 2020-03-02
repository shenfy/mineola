#include "prefix.h"
#include "../include/Material.h"
#include <glm/gtc/type_ptr.hpp>
#include "../include/GLEffect.h"
#include "../include/Engine.h"
#include "../include/Texture.h"
#include "../include/glutility.h"

namespace mineola {

void Material::UploadToShader(std::shared_ptr<GLEffect> &effect) {
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
  int32_t tex_unit = 0;
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
    if (all_found)
      effect->UploadVariable(iter->first.c_str(), &tex_units[0]);
 }
}

} //namespace
