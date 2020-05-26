#ifndef CROP_ENTITY_H
#define MINEOLA_TEXTUREMIXER_H

#include <vector>
#include <memory>
#include <mineola/Entity.h>
#include <imgpp/imgpp_bc.hpp>

namespace mineola {

class TextureMixer : public mineola::Entity {
public:
  TextureMixer(const char *tex_name,
    const imgpp::BCImgROI &src_roi, const imgpp::BCImgROI &dst_roi);

  virtual void FrameMove(double time, double frame_time) override;

private:
  std::string tex_name_;
  imgpp::BCImgROI src_roi_;
  imgpp::BCImgROI dst_roi_;
};

}
#endif
