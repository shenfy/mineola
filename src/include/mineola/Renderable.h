#ifndef MINEOLA_RENDERABLE_H
#define MINEOLA_RENDERABLE_H

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "VertexType.h"
#include "BasisObj.h"
#include "Skin.h"
#include "AABB.h"

namespace mineola {

class Renderable : public Resource {
public:
  Renderable();
  Renderable(int16_t queue_id);
  virtual ~Renderable();

  virtual void PreRender(double frame_time, uint32_t pass);
  virtual void Draw(double frame_time, uint32_t pass);

  void AddVertexArray(std::shared_ptr<vertex_type::VertexArray> va,
            const char *material_name);
  void SetEffect(const char *effect_name);
  inline const char *GetEffectName() const {return effect_name_.c_str();}

  size_t NumVertexArray() const;
  void SetMaterial(size_t index, const char *material_name);
  const std::string &GetMaterialName(size_t index) const;

  int LayerMask() const;
  void SetLayerMask(int layer_mask);

  int16_t QueueId() const;
  void SetQueueId(int16_t id);

  void SetSkin(std::shared_ptr<Skin> skin);

  void SetBbox(const AABB &bbox);
  const std::optional<AABB> &Bbox() const;

  enum {
    kQueueOpaque = 0,
    kQueueTransparent = 1024
  };

protected:
  int16_t q_id_;
  int layer_mask_;
  std::string effect_name_; //for standard render pass
  std::vector<std::shared_ptr<vertex_type::VertexArray> > vertex_arrays_;
  std::vector<std::string> material_names_;
  std::shared_ptr<Skin> skin_;
  std::optional<AABB> bbox_;
};

} //namespaces

#endif
