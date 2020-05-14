#include "../include/TextureMixer.h"
#include <mineola/Engine.h>
#include <mineola/Texture.h>
#include <mineola/TextureDesc.h>

namespace mineola {

TextureMixer::TextureMixer(const char *tex_name,
  const imgpp::BCImgROI &src_roi, const imgpp::BCImgROI &dst_roi)
  : tex_name_(tex_name), src_roi_(src_roi), dst_roi_(dst_roi) {
}

void TextureMixer::FrameMove(double time, double frame_time) {
  static uint32_t count = 0;
  static uint32_t shift = 0;
  if (count > 5) {
    for (int block_y = 0; block_y < src_roi_.VerticalBlockNum(); ++block_y) {
      auto dst_sub_roi = dst_roi_.SubRegion(
        0, block_y * dst_roi_.BlockHeight(), 0,
        dst_roi_.Width() - 1, (block_y + 1) * dst_roi_.BlockHeight() - 1, 0);
      auto src_sub_roi = src_roi_.SubRegion(
        0, (block_y + shift) %  src_roi_.VerticalBlockNum() * src_roi_.BlockHeight(), 0,
        src_roi_.Width() - 1, (block_y + shift + 1) %  src_roi_.VerticalBlockNum() * src_roi_.BlockHeight() - 1, 0);

      imgpp::CopyData(dst_sub_roi, src_sub_roi);
    }
    auto tex = std::dynamic_pointer_cast<mineola::Texture2D>(
      mineola::Engine::Instance().ResrcMgr().Find(tex_name_));
    if (tex) {
      mineola::SubTextureDesc desc;
      desc.level = 0;
      desc.x_offset = 0;
      desc.y_offset = 0;
      desc.width = tex->Desc().width;
      desc.height = tex->Desc().height;
      desc.data_size = dst_roi_.VerticalBlockNum() * dst_roi_.HorizontalBlockNum() * dst_roi_.BlockBytes();
      tex->UpdateTexture(desc, dst_roi_.GetData());
    }
    shift++;
    if (shift == src_roi_.VerticalBlockNum()) {
      shift = 0;
    }
    count = 0;
  }
  count++;
}
}
