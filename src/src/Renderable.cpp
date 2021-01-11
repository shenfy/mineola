#include "prefix.h"
#include <mineola/Renderable.h>
#include <mineola/Engine.h>
#include <mineola/Material.h>
#include <mineola/VertexType.h>

namespace mineola {

Renderable::Renderable() :
  q_id_(kQueueOpaque),
  layer_mask_(RenderPass::RENDER_LAYER_0) {
}

Renderable::Renderable(int16_t queue_id) :
  q_id_(queue_id),
  layer_mask_(RenderPass::RENDER_LAYER_0) {
}

Renderable::~Renderable() {
  vertex_arrays_.clear();
}

void Renderable::SetLayerMask(int layer_mask) {
  layer_mask_ = layer_mask;
}

int Renderable::LayerMask() const {
  return layer_mask_;
}

int16_t Renderable::QueueId() const {
  return q_id_;
}

void Renderable::SetQueueId(int16_t queue_id) {
  q_id_ = queue_id;
}

void Renderable::SetSkin(std::shared_ptr<Skin> skin) {
  skin_ = std::move(skin);
}

void Renderable::AddVertexArray(
  std::shared_ptr<vertex_type::VertexArray> va,
  const char *material_name) {

  vertex_arrays_.push_back(std::move(va));
  material_names_.push_back(material_name);
}

void Renderable::SetEffect(const char *effect_name) {
  effect_name_ = effect_name;
}

void Renderable::SetShadowmapEffect(std::string effect_name) {
  shadowmap_effect_name_ = std::move(effect_name);
}

std::optional<const char *> Renderable::GetShadowmapEffectName() const {
  if (shadowmap_effect_name_) {
    return shadowmap_effect_name_->c_str();
  } else {
    return std::nullopt;
  }
}

size_t Renderable::NumVertexArray() const {
  return vertex_arrays_.size();
}

void Renderable::SetMaterial(size_t index, const char *material_name) {
  material_names_[index] = material_name;
}

const std::string &Renderable::GetMaterialName(size_t index) const {
  return material_names_[index];
}

void Renderable::SetBbox(const AABB &bbox) {
  bbox_ = std::optional<AABB>{bbox};
}

const std::optional<AABB> &Renderable::Bbox() const {
  return bbox_;
}

void Renderable::PreRender(double frame_time, uint32_t pass_idx) {
  auto &en = Engine::Instance();
  auto &pass = en.RenderPasses()[pass_idx];
  switch (pass.sfx) {
  case RenderPass::SFX_PASS_SHADOWMAP:
    en.ChangeEffect(*shadowmap_effect_name_, false);
    break;
  default:
    en.ChangeEffect(effect_name_, false);
    break;
  }

  if (skin_) {
    skin_->PreRender(frame_time, pass_idx);
  }
}

void Renderable::Draw(double frame_time, uint32_t pass) {
  Engine &en = Engine::Instance();
  for (uint32_t i = 0; i < vertex_arrays_.size(); ++i) {
    en.DoRender(*vertex_arrays_[i], material_names_[i]);
  }
}

}
