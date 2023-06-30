#define GLM_FORCE_CTOR_INIT
#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mineola/Imgpp.hpp>
#include <mineola/ImgppLoaders.hpp>
#include <mineola/ImgppLoadersExt.hpp>
#include <mineola/ImgppCompositeImg.hpp>
#include <mineola/SH3.h>
#include "EnvBRDF.h"

namespace {
  enum { kLUTSize = 128 };
  static const float kPi = glm::pi<float>();

  // WARNING: does not handle any overflow/underflow etc.
  uint16_t Float2Half(float val) {
    uint32_t x = *((uint32_t *)&val);
    return ((x >> 16) & 0x8000) | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((x >> 13) & 0x03ff);
  }

  mineola::imgpp::Img Float2Half(const mineola::imgpp::ImgROI &roi) {
    mineola::imgpp::Img output;
    output.SetSize(roi.Width(), roi.Height(), roi.Depth(), roi.Channel(),
                   16, true, true, 4);
    auto &out_roi = output.ROI();
    for (uint32_t y = 0; y < roi.Height(); y++) {
      for (uint32_t x = 0; x < roi.Width(); x++) {
        for (uint32_t c = 0; c < roi.Channel(); c++) {
          out_roi.At<uint16_t>(x, y, 0, c) = Float2Half(roi.At<float>(x, y, 0, c));
        }
      }
    }

    return output;
  }

}

namespace mineola {
  namespace utils {

    bool PrefilterLightProbe(const std::string &in_fn, const std::string &out_fn,
                             bool mirror, bool use_half_flt) {

      mineola::imgpp::Img img;
      if (!mineola::imgpp::Load(in_fn.c_str(), img, false)) {
        return false;
      }

      {
        auto &roi = img.ROI();
        if (roi.Channel() != 3 || !roi.IsFloat()) {
          std::cerr << "Only 3 channel float images are supported!" << std::endl;
          return false;
        }
      }

      {
        if (mirror) {
          // mirror the image horizontally
          auto &roi = img.ROI();
          auto width = roi.Width();
          auto height = roi.Height();
          for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width / 2; ++x) {
              using std::swap;
              swap(roi.At<glm::vec3>(x, y), roi.At<glm::vec3>(width - 1 - x, y));
            }
          }
        }
      }

      std::unordered_map<std::string, std::string> dict;

      ////////////////////////////////////////////////////////////////////////////////////
      // build pyramid
      auto pyramid = BuildPyramid(img);
      auto &roi_0 = pyramid[0].ROI();

      ////////////////////////////////////////////////////////////////////////////////////
      // diffuse shading
      auto width = roi_0.Width();
      auto height = roi_0.Height();

      std::vector<glm::vec3> diffuse_coeffs(math::SH3<>::kNumCoeffs);
      {
        std::vector<float> basis(math::SH3<>::kNumCoeffs);
        glm::vec3 normal;

        for (uint32_t row = 0; row < height; row++) {
          // the image should be loaded bottom first
          float theta = (row + 0.5f) / height * kPi;
          float weight = (2.f * kPi / width) * (kPi / height) * std::sin(theta);

          for (uint32_t col = 0; col < width; col++) {
            // translate to (-pi, pi)
            float phi = (col + 0.5f) / width * kPi * 2.f - kPi;
            math::SphToCart(theta, phi, normal);
            auto l = roi_0.Pixel<glm::vec3>(col, row);
            math::SH3Basis(normal, basis);

            for (int idx = 0; idx < math::SH3<>::kNumCoeffs; ++idx) {
              diffuse_coeffs[idx] += l * weight * basis[idx];
            }
          }
        }
      }

      {
        std::string sh_buffer;
        sh_buffer.resize(sizeof(glm::vec3) * diffuse_coeffs.size());
        char *p = (char *)sh_buffer.data();
        for (size_t i = 0; i < diffuse_coeffs.size(); ++i) {
          memcpy(p, glm::value_ptr(diffuse_coeffs[i]), sizeof(glm::vec3));
          p += sizeof(glm::vec3);
        }
        dict["sh"] = std::move(sh_buffer);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      // specular shading
      std::vector<mineola::imgpp::ImgROI> rois;
      for (auto &img : pyramid) {
        rois.push_back(img.ROI());
      }

      auto levels = (int)std::floor(std::log2((float)width)) + 1;
      std::vector<mineola::imgpp::Img> filtered_maps(levels);
      {
        glm::vec3 normal;

        auto level_w = width;
        auto level_h = height;
        for (int l = 0; l < levels; l++) {
          auto &map = filtered_maps[l];
          map.SetSize(level_w, level_h, 1, 3, sizeof(float) << 3, true, true);
          // float roughness = pow(2.f, (l - levels + 4.f) / 1.15f);
          float roughness = (float)l / (levels - 1); // l = sqrt(r) * mip_count

          if (roughness <= 1.f) {
            for (uint32_t y = 0; y < level_h; ++y) {
              float theta = (y + 0.5f) / level_h * kPi;

              for (uint32_t x = 0; x < level_w; ++x) {
                float phi = (x + 0.5f) / level_w * kPi * 2.f - kPi;
                math::SphToCart(theta, phi, normal);

                map.ROI().At<glm::vec3>(x, y) =
                    PrefilterSpecularEnvMap(roughness, normal, rois);
              }
            }
          }

          level_w = std::max(level_w / 2, 1u);
          level_h = std::max(level_h / 2, 1u);
        }
      }

      mineola::imgpp::CompositeImg out_img;
      if (use_half_flt) {
        mineola::imgpp::TextureDesc desc{mineola::imgpp::FORMAT_RGB16_SFLOAT_PACK16, mineola::imgpp::TARGET_2D, true};
        out_img.SetSize(desc, (uint32_t)levels, 1, 1, width, height, 1, 4);
        for (int l = 0; l < levels; l++) {
          auto img_half = Float2Half(filtered_maps[l].ROI());
          out_img.AddBuffer(img_half.Data());
          out_img.SetData(img_half.ROI().GetData(), l, 0, 0);
        }
      } else {
        // write all probe images to one single KTX file
        mineola::imgpp::TextureDesc desc{mineola::imgpp::FORMAT_RGB32_SFLOAT_PACK32, mineola::imgpp::TARGET_2D, true};
        out_img.SetSize(desc, (uint32_t)levels, 1, 1, width, height, 1, 4);
        for (int l = 0; l < levels; l++) {
          out_img.AddBuffer(filtered_maps[l].Data());
          out_img.SetData(filtered_maps[l].ROI().GetData(), l, 0, 0);
        }
      }

      return mineola::imgpp::WriteKTX(out_fn.c_str(), out_img, dict, false);
    }

  }
} // namespaces

int main(int argc, const char *argv[]) {

  if (argc < 3) {
    std::cout << "Usage: envlight (input fn) (output fn) [-nomirror] [-fp16]" << std::endl;
    return 0;
  }

  auto in_fn = std::string(argv[1]);
  auto out_fn = std::string(argv[2]);

  bool mirror = true;
  bool use_half_flt = false;

  for (int idx = 3; idx < argc; idx++) {
    auto param = std::string(argv[idx]);
    if (param == "-nomirror") {
      std::cout << "nomirror" << std::endl;
      mirror = false;
    } else if (param == "-fp16") {
      std::cout << "fp16" << std::endl;
      use_half_flt = true;
    }
  }

  if (!mineola::utils::PrefilterLightProbe(in_fn, out_fn, mirror, use_half_flt))
  {
    std::cerr << "Error: failed to prefilter envmap!" << std::endl;
    return 1;
  }

  return 0;
}
