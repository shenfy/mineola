#ifndef MINEOLA_ENVLIGHT_H
#define MINEOLA_ENVLIGHT_H

#include "Light.h"
#include "SH3.h"

namespace mineola {

class Texture;
class Texture2D;
class ImgppTextureSrc;

class EnvLight : public Light {
public:
  EnvLight(size_t idx);
  ~EnvLight() override;

  void UpdateLightTransform(const math::Rbt &rbt) override;
  void UpdateUniforms(UniformBlock *ub) override;

  bool LoadFromFile(const char *fn);

  bool LoadFromFolder(const char *path);

private:
  bool CreateLightProbeTexture(std::shared_ptr<ImgppTextureSrc> tex_src);

protected:

  glm::mat4 mat_;
  math::SH3<glm::vec4> sh3_; // use vec4 for passing to uniform block, actual data is vec3
  std::shared_ptr<Texture2D> light_probe_;
};

}

#endif
