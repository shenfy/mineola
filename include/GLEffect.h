#ifndef MINEOLA_GLEFFECT_H
#define MINEOLA_GLEFFECT_H

#include <vector>
#include "GLProgram.h"

namespace mineola {

class GLShader;
struct RenderState;

class GLEffect : public GLProgram {
public:
  GLEffect();
  virtual ~GLEffect();

  bool AttachShaders(std::shared_ptr<GLShader> &pVS,
		std::shared_ptr<GLShader> &pPS);

  bool FindAttribLoc(const char *semantics, uint32_t format, uint32_t length, uint32_t *loc) const;

  void ApplyRenderStates() const;
  void SetRenderStates(std::vector<std::unique_ptr<RenderState>> states);

protected:
  // IMPORTANT: so far the vertex shader's input attribute layout indices need to be manually assigned in GLSL,
  // following the index order specified in the VertexType::GetSemanticsBindLocation().
  void ReorderAttribBindings();
  bool GenerateAttribMap();

  std::shared_ptr<GLShader> vertex_shader_;
  std::shared_ptr<GLShader> pixel_shader_;

  //map from var/attrib name to location and type
  std::unordered_map<std::string,
    std::tuple<uint32_t/*loc*/, uint32_t/*type*/, uint32_t/*size*/>> attribute_map_;
  std::vector<std::unique_ptr<RenderState>> render_states_;
};

typedef std::vector<std::pair<std::string, std::string>> effect_defines_t;

bool CreateEffectFromFileHelper(const char *effectname,
  const char *filename_vs, const char *filename_fs,
  const effect_defines_t *defines, std::vector<std::unique_ptr<RenderState>> states);
bool CreateEffectFromMemHelper(const char *effectname, const char *vs_buf, const char *ps_buf,
  const effect_defines_t *defines, std::vector<std::unique_ptr<RenderState>> states);

bool ReloadEffectFromFileHelper(const char *effectname,
  const char *filename_vs, const char *filename_fs,
  const effect_defines_t *defines);
bool ReloadEffectFromMemHelper(const char *effectname, const char *vs_buf, const char *ps_buf,
  const effect_defines_t *defines);

} //namespace

#endif
