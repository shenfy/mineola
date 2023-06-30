#include "EnvBRDF.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace {

enum { kBRDFSamples = 1024, kEnvMapSamples = 128 };

static const float kPi = glm::pi<float>();
static const float kRcpPi = 1.0f / kPi;

float RadicalInverse(uint32_t bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

inline glm::vec3 SampleLatLonMap(const mineola::imgpp::ImgROI &roi, float theta, float phi) {

  auto w = (int)roi.Width();
  auto h = (int)roi.Height();

  float x = (phi + kPi) * kRcpPi * 0.5f * w - 0.5f;
  float y = theta * kRcpPi * h - 0.5f;

  float x0 = floor(x), y0 = floor(y);
  float dx = x - x0, dy = y - y0;
  auto x0i = (int)std::max(x0, 0.f), y0i = (int)std::max(y0, 0.f);
  auto x1i = std::min((int)x0 + 1, w - 1), y1i = std::min((int)y0 + 1, h - 1);

  auto v0 = roi.At<glm::vec3>(x0i, y0i);
  auto v1 = roi.At<glm::vec3>(x1i, y0i);
  auto v2 = roi.At<glm::vec3>(x0i, y1i);
  auto v3 = roi.At<glm::vec3>(x1i, y1i);
  auto vx0 = v0 * (1.f - dx) + v1 * dx;
  auto vx1 = v2 * (1.f - dx) + v3 * dx;
  auto result = vx0 * (1.f - dy) + vx1 * dy;
  return result;
}

}  // namespace

namespace mineola {

namespace math {

// mineola convention
// Y is up
void SphToCart(float theta, float phi, glm::vec3 &cart) {
  cart.y = std::cos(theta);
  auto sin_theta = std::sin(theta);
  cart.z = sin_theta * std::cos(phi);
  cart.x = sin_theta * std::sin(phi);
  cart = glm::normalize(cart);
}

void CartToSph(const glm::vec3 &cart, float &theta, float &phi) {
  theta = std::acos(cart.y);
  phi = atan2(cart.x, cart.z);
}

}  // namespace math

namespace utils {

glm::vec2 Hammersley(uint32_t i, uint32_t num_samples) {
  return glm::vec2(
    float(i) / num_samples,
    RadicalInverse(i));
}

// build fast orthonormal basis using https://graphics.pixar.com/library/OrthonormalB/paper.pdf
// changed from (N == z) to (N == y)
glm::mat3 TangentBasis(const glm::vec3 &N) {
  float sign = N.y >= 0.f ? 1.f : -1.f;
  float a = -1.f / (sign + N.y);
  float b = N.z * N.x * a;

  auto b1 = glm::normalize(glm::vec3(
    sign * b,
    -sign * N.z,
    1 + sign * a * N.z * N.z));
  auto b2 = glm::normalize(glm::vec3(
    sign + a * N.x * N.x,
    -N.x,
    b));

  return glm::mat3(b2, N, b1);
}

glm::vec3 Tangent2World(const glm::vec3 &v, const glm::vec3 &N) {
  return TangentBasis(N) * v;
}

glm::vec4 ImportanceSampleGGX(const glm::vec2 &Xi, float roughness) {
  auto a = roughness * roughness;
  auto a2 = a * a;

  // phi in [-π, π], theta in [0, π]
  float phi = 2 * kPi * Xi.x - kPi;
  float cos_theta = sqrt((1.f - Xi.y) / (1.f + (a2 - 1.f) * Xi.y));
  float sin_theta = sqrt(1.f - std::min(1.f, cos_theta * cos_theta));

  auto H = glm::vec3(sin_theta * sin(phi), cos_theta, sin_theta * cos(phi));
  H = glm::normalize(H);

  auto d = (a2 - 1.f) * cos_theta * cos_theta + 1.f;
  auto D = a2 / (kPi * d * d);
  auto pdf = D * cos_theta;

  return glm::vec4(H, pdf);
}

float G_GGX(float roughness, float NoV, float NoL) {
  auto k = (roughness + 1.f) * (roughness + 1.f) / 8.f;
  auto gl = NoL / (NoL * (1.f - k) + k);
  auto gv = NoV / (NoV * (1.f - k) + k);
  return gl * gv;
}

float G_GGX_Approx(float a, float NoV, float NoL) {
  auto gv = NoL * (NoV * (1.f - a) + a);
  auto gl = NoV * (NoL * (1.f - a) + a);
  return 0.5f / (gv + gl);
}

// https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
glm::vec2 IntegrateBRDF(float roughness, float NoV) {

  // auto V = glm::vec3(sqrt(1.f - NoV * NoV), 0.f, NoV);
  auto V = glm::vec3(0.f, NoV, sqrt(1.f - NoV * NoV));
  V = glm::normalize(V);
  float A = 0.f;
  float B = 0.f;
  float C = 0.f;
  float a = roughness * roughness;
  float a2 = a * a;

  for (int i = 0; i < kBRDFSamples; ++i) {
    auto Xi = Hammersley((uint32_t)i, (uint32_t)kBRDFSamples);
    auto H = glm::vec3(ImportanceSampleGGX(Xi, roughness));
    auto L = glm::normalize(2.f * glm::dot(V, H) * H - V);

    float NoL = glm::clamp(L.y, 0.f, 1.f);
    float NoH = glm::clamp(H.y, 0.f, 1.f);
    float VoH = glm::clamp(glm::dot(V, H), 0.f, 1.f);

    if (NoL > 0.f) {
      auto G = G_GGX_Approx(a, NoV, NoL);
      auto G_pdf = NoL * G * (4.f * VoH / NoH);

      auto Fc = pow(1.f - VoH, 5.f);
      A += (1 - Fc) * G_pdf;
      B += Fc * G_pdf;
    }
  }

  return glm::vec2(A, B) / (float)kBRDFSamples;
}

glm::vec3 PrefilterSpecularEnvMap(
  float roughness,
  const glm::vec3 &N,
  const std::vector<mineola::imgpp::ImgROI> &pyramid) {

  glm::vec3 prefiltered_color;
  float total_weight = 0.f;
  float theta = 0.f, phi = 0.f;

  auto mip_count = (float)pyramid.size();
  auto w0 = pyramid[0].Width();
  auto h0 = pyramid[0].Height();

  for (int i = 0; i < kEnvMapSamples; ++i) {
    auto Xi = Hammersley((uint32_t)i, (uint32_t)kEnvMapSamples);
    auto sample = ImportanceSampleGGX(Xi, roughness);
    auto H = glm::vec3(sample);
    auto pdf = sample.w / (4.f * kPi);
    auto omega_s = 1.f / (kEnvMapSamples * pdf);
    auto omega_p = 4.f * kPi * H.y / (w0 * h0);  // approx distortion
    auto mip_level = glm::clamp(0.5f * std::log2(omega_s / omega_p), 0.f, mip_count);

    H = Tangent2World(H, N);

    auto L = glm::normalize(2.f * dot(N, H) * H - N);

    math::CartToSph(L, theta, phi);

    auto NoL = glm::clamp(glm::dot(N, L), 0.f, 1.f);
    if (NoL > 0.f) {


      auto sample = SampleLatLonMap(pyramid[mip_level], theta, phi);
      prefiltered_color += sample;
      total_weight += NoL;
    }
  }

  if (total_weight > 1e-3) {
    prefiltered_color /= total_weight;
  }

  return prefiltered_color;
}

void HalfImage(const mineola::imgpp::ImgROI &src, mineola::imgpp::ImgROI &dst) {
  auto w_src = src.Width(), h_src = src.Height();
  auto w = dst.Width(), h = dst.Height();

  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      auto x0 = x * 2, y0 = y * 2;
      auto x1 = std::min(w_src - 1, x0 + 1);
      auto y1 = std::min(h_src - 1, y0 + 1);

      dst.At<glm::vec3>(x, y) = (src.At<glm::vec3>(x0, y0)
        + src.At<glm::vec3>(x1, y0)
        + src.At<glm::vec3>(x0, y1)
        + src.At<glm::vec3>(x1, y1)) * 0.25f;
    }
  }
}

std::vector<mineola::imgpp::Img> BuildPyramid(mineola::imgpp::Img &input_img) {

  auto &input_roi = input_img.ROI();
  auto width = input_roi.Width();
  auto height = input_roi.Height();

  int levels = (int)std::floor(std::log2((float)width)) + 1;

  std::vector<mineola::imgpp::Img> pyramid(levels);

  auto current_w = width;
  auto current_h = height;
  pyramid[0] = input_img;  // copy level 0

  for (int l = 1; l < levels; ++l) {
    current_h = std::max(current_h / 2, 1u);
    current_w = std::max(current_w / 2, 1u);

    auto &img = pyramid[l];
    img.SetSize(current_w, current_h, 1, 3, 32, true, true);
    auto &roi = img.ROI();

    HalfImage(pyramid[l - 1].ROI(), roi);
  }

  return pyramid;
}



}}  // end namespaces