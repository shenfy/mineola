#include "prefix.h"
#include <mineola/GLTFLoader.h>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <fx/gltf.h>
#include <glm/glm.hpp>
#include <mineola/Material.h>
#include <mineola/Engine.h>
#include <mineola/Renderable.h>
#include <mineola/TextureHelper.h>
#include <mineola/GraphicsBuffer.h>
#include <mineola/SceneNode.h>
#include <mineola/UniformWrappers.h>
#include <mineola/PBRShaders.h>
#include <mineola/AnimatedEntity.h>
#include <mineola/GLMHelper.h>

namespace {

using namespace mineola;
using namespace mineola::vertex_type;

int MapGLTFSemantics(const std::string &semantics_str) {
  if (semantics_str == "POSITION") {
    return POSITION;
  } else if (semantics_str == "NORMAL") {
    return NORMAL;
  } else if (semantics_str == "TANGENT") {
    return TANGENT;
  } else if (semantics_str == "TEXCOORD_0") {
    return TEXCOORD0;
  } else if (semantics_str == "TEXCOORD_1") {
    return TEXCOORD1;
  } else if (semantics_str == "COLOR_0") {
    return DIFFUSE_COLOR;
  } else if (semantics_str == "JOINTS_0") {
    return BLEND_INDEX;
  } else if (semantics_str == "WEIGHTS_0") {
    return BLEND_WEIGHT;
  }
  return UNKNOWN;
}

int MapGLTFComponentType(fx::gltf::Accessor::ComponentType comp_type) {
  switch (comp_type) {
    case fx::gltf::Accessor::ComponentType::Byte:
      return type_mapping::BYTE;
    case fx::gltf::Accessor::ComponentType::UnsignedByte:
      return type_mapping::UBYTE;
    case fx::gltf::Accessor::ComponentType::Short:
      return type_mapping::INT16;
    case fx::gltf::Accessor::ComponentType::UnsignedShort:
      return type_mapping::UINT16;
    case fx::gltf::Accessor::ComponentType::UnsignedInt:
      return type_mapping::UINT32;
    case fx::gltf::Accessor::ComponentType::Float:
      return type_mapping::FLOAT32;
    default:
      return type_mapping::UNKNOWN;
  }
}

int MapGLTFVecLength(fx::gltf::Accessor::Type type) {
  switch (type) {
    case fx::gltf::Accessor::Type::Scalar:
      return 1;
    case fx::gltf::Accessor::Type::Vec2:
      return 2;
    case fx::gltf::Accessor::Type::Vec3:
      return 3;
    case fx::gltf::Accessor::Type::Vec4:
      return 4;
    case fx::gltf::Accessor::Type::Mat2:
      return 4;
    case fx::gltf::Accessor::Type::Mat3:
      return 9;
    case fx::gltf::Accessor::Type::Mat4:
      return 16;
    default:
      return 0;
  }
}

uint32_t MapGLTFMagFilter(fx::gltf::Sampler::MagFilter type) {
  switch (type) {
    case fx::gltf::Sampler::MagFilter::Nearest:
      return TextureDesc::kNearest;
    case fx::gltf::Sampler::MagFilter::Linear:
      return TextureDesc::kLinear;
    default:
      return TextureDesc::kLinear;
  }
}

uint32_t MapGLTFMinFilter(fx::gltf::Sampler::MinFilter type) {
  switch (type) {
    case fx::gltf::Sampler::MinFilter::Nearest:
      return TextureDesc::kNearest;
    case fx::gltf::Sampler::MinFilter::Linear:
      return TextureDesc::kLinear;
    case fx::gltf::Sampler::MinFilter::NearestMipMapNearest:
      return TextureDesc::kNearestMipmapNearest;
    case fx::gltf::Sampler::MinFilter::LinearMipMapNearest:
      return TextureDesc::kLinearMipmapNearest;
    case fx::gltf::Sampler::MinFilter::NearestMipMapLinear:
      return TextureDesc::kNearestMipmapLinear;
    case fx::gltf::Sampler::MinFilter::LinearMipMapLinear:
      return TextureDesc::kLinearMipmapLinear;
    default:
      return TextureDesc::kLinearMipmapLinear;
  }
}

uint32_t MapGLTFWrapMode(fx::gltf::Sampler::WrappingMode mode) {
  switch (mode) {
    case fx::gltf::Sampler::WrappingMode::ClampToEdge:
      return TextureDesc::kClampToEdge;
    case fx::gltf::Sampler::WrappingMode::MirroredRepeat:
      return TextureDesc::kMirrorRepeat;
    case fx::gltf::Sampler::WrappingMode::Repeat:
      return TextureDesc::kRepeat;
    default:
      return TextureDesc::kRepeat;
  }
}

std::string AbbrevTextureMode(
  uint32_t min_filter, uint32_t mag_filter, uint32_t wrap_s, uint32_t wrap_t) {
  std::stringstream ss;
  ss << min_filter << mag_filter << wrap_s << wrap_t;
  return ss.str();
}

void SetAttribFlag(int semantics, AttribFlags &flags) {
  switch (semantics) {
  case NORMAL:
    flags.EnableNormal();
    break;
  case TANGENT:
    flags.EnableTangent();
    break;
  case TEXCOORD0:
    flags.EnableTexCoord();
    break;
  case TEXCOORD1:
    flags.EnableTexCoord2();
    break;
  case DIFFUSE_COLOR:
    flags.EnableColor();
    break;
  default:
    break;
  }
}

enum class BufferViewUsage {
  kUnknown = -1,
  kTexture = 1,
  kCPU,
  kVBO = GL_ARRAY_BUFFER,
  kIBO = GL_ELEMENT_ARRAY_BUFFER
};

BufferViewUsage MapGLTFTargetType(fx::gltf::BufferView::TargetType type) {
  switch (type) {
    case fx::gltf::BufferView::TargetType::ArrayBuffer:
      return BufferViewUsage::kVBO;
    case fx::gltf::BufferView::TargetType::ElementArrayBuffer:
      return BufferViewUsage::kIBO;
    default:
      return BufferViewUsage::kUnknown;
  }
}

std::vector<float> ParseNormalizedFloatBuffer(const fx::gltf::Document &doc, int acc_id) {
  auto &acc = doc.accessors[acc_id];
  auto &bv = doc.bufferViews[acc.bufferView];
  auto &buffer = doc.buffers[bv.buffer];
  uint32_t offset = bv.byteOffset + acc.byteOffset;

  int num_vals = MapGLTFVecLength(acc.type) * acc.count;
  uint32_t element_size = type_mapping::SizeOf(MapGLTFComponentType(acc.componentType));

  std::vector<float> result;
  result.resize(num_vals);
  const uint8_t *start_ptr = &buffer.data[offset];
  for (int i = 0; i < num_vals; ++i) {
    const uint8_t *ptr = start_ptr + element_size * i;
    float val = 0.0f;
    switch (acc.componentType) {
      // conversion formulas come from glTF 2.0 specification
      case fx::gltf::Accessor::ComponentType::Byte: {
        val = std::max(*ptr / 127.0f, -1.0f);
        break;
      }
      case fx::gltf::Accessor::ComponentType::UnsignedByte: {
        val = *ptr / 255.0f;
        break;
      }
      case fx::gltf::Accessor::ComponentType::Short: {
        val = std::max(*(int16_t*)ptr / 32767.0f, -1.0f);
        break;
      }
      case fx::gltf::Accessor::ComponentType::UnsignedShort: {
        val = *(uint16_t*)ptr / 65535.0f;
        break;
      }
      case fx::gltf::Accessor::ComponentType::Float: {
        val = *(float*)ptr;
        break;
      }
      default:
        break;
    }
    result[i] = val;
  }

  return result;
}

void ParseAnimationChannel(const fx::gltf::Document &doc,
  const fx::gltf::Animation::Channel &ch,
  int acc_in, int acc_out,
  animation::Channel &channel) {

  // load buffer to vector<float>
  auto timestamps = ParseNormalizedFloatBuffer(doc, acc_in);
  auto flts = ParseNormalizedFloatBuffer(doc, acc_out);

  // parse floats to irregular keyframes
  std::vector<animation::KeyFrame> raw_key_frames;
  std::vector<animation::KeyFrame> raw_in_tangents;
  std::vector<animation::KeyFrame> raw_out_tangents;

  bool read_tangents = false;
  size_t count_multiplier = 1;
  if (channel.interp == animation::Channel::kInterpCubicSpline) {
    read_tangents = true;
    count_multiplier = 3;
  }


  if (ch.target.path == "translation") {
    channel.type = animation::Channel::kAnimTranslation;
    size_t elem_size = count_multiplier * 3;
    raw_key_frames.resize(flts.size() / elem_size);

    if (read_tangents) {
      raw_in_tangents.resize(raw_key_frames.size());
      raw_out_tangents.resize(raw_key_frames.size());
    }

    for (size_t idx = 0; idx < raw_key_frames.size(); ++idx) {
      if (read_tangents) {
        raw_in_tangents[idx].translation = {
          flts[idx * elem_size], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
        raw_key_frames[idx].translation = {
          flts[idx * elem_size + 3], flts[idx * elem_size + 4], flts[idx * elem_size + 5]
        };
        raw_out_tangents[idx].translation = {
          flts[idx * elem_size + 6], flts[idx * elem_size + 7], flts[idx * elem_size + 8]
        };
      } else {
        raw_key_frames[idx].translation = {
          flts[idx * elem_size], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
      }
    }
  } else if (ch.target.path == "rotation") {
    channel.type = animation::Channel::kAnimRotation;
    size_t elem_size = count_multiplier * 4;
    raw_key_frames.resize(flts.size() / elem_size);
    if (read_tangents) {
      raw_in_tangents.resize(raw_key_frames.size());
      raw_out_tangents.resize(raw_key_frames.size());
    }

    for (size_t idx = 0; idx < raw_key_frames.size(); ++idx) {
      if (read_tangents) {
        raw_in_tangents[idx].rotation = {
          flts[idx * elem_size + 3],
          flts[idx * elem_size + 0], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
        raw_key_frames[idx].rotation = {
          flts[idx * elem_size + 7],
          flts[idx * elem_size + 4], flts[idx * elem_size + 5], flts[idx * elem_size + 6]
        };
        raw_out_tangents[idx].rotation = {
          flts[idx * elem_size + 11],
          flts[idx * elem_size + 8], flts[idx * elem_size + 9], flts[idx * elem_size + 10]
        };
      } else {
        raw_key_frames[idx].rotation = {
          flts[idx * elem_size + 3],
          flts[idx * elem_size + 0], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
      }
    }
  } else if (ch.target.path == "scale") {
    channel.type = animation::Channel::kAnimScale;
    size_t elem_size = count_multiplier * 3;
    raw_key_frames.resize(flts.size() / elem_size);
    if (read_tangents) {
      raw_in_tangents.resize(raw_key_frames.size());
      raw_out_tangents.resize(raw_key_frames.size());
    }

    for (size_t idx = 0; idx < raw_key_frames.size(); ++idx) {
      if (read_tangents) {
        raw_in_tangents[idx].scale = {
          flts[idx * elem_size], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
        raw_key_frames[idx].scale = {
          flts[idx * elem_size + 3], flts[idx * elem_size + 4], flts[idx * elem_size + 5]
        };
        raw_out_tangents[idx].scale = {
          flts[idx * elem_size + 6], flts[idx * elem_size + 7], flts[idx * elem_size + 8]
        };
      } else {
        raw_key_frames[idx].scale = {
          flts[idx * elem_size], flts[idx * elem_size + 1], flts[idx * elem_size + 2]
        };
      }
    }
  }

  // resample key frames
  float time_point = 0.0f;
  while (true) {
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), time_point);
    if (it == timestamps.end()) {  // out of range, repeat last and exit
      channel.key_frames.push_back(raw_key_frames.back());
      break;
    }

    if (it == timestamps.begin()) {
      channel.key_frames.push_back(raw_key_frames.front());
    } else {
      auto prev_it = it - 1;
      auto prev_idx = prev_it - timestamps.begin();

      if (channel.interp == animation::Channel::kInterpStep) {
        channel.key_frames.push_back(raw_key_frames[prev_idx]);
      } else if (channel.interp == animation::Channel::kInterpLinear) {
        auto time0 = *prev_it;
        auto time1 = *it;
        float t = (time_point - time0) / (time1 - time0);
        channel.key_frames.push_back(animation::KeyFrame::Lerp(
          raw_key_frames[prev_idx], raw_key_frames[prev_idx + 1], t));
      } else if (channel.interp == animation::Channel::kInterpCubicSpline) {
        auto time0 = *prev_it;
        auto time1 = *it;
        float interval = time1 - time0;
        float t = (time_point - time0) / interval;
        channel.key_frames.push_back(animation::KeyFrame::CubicSpline(
          raw_key_frames[prev_idx], raw_out_tangents[prev_idx],
          raw_key_frames[prev_idx + 1], raw_in_tangents[prev_idx + 1],
          interval, t));
      }
    }

    time_point += 0.04f;  // 0.1s increment
  }
}

bool CreateSceneFromGLTFDoc(
  const fx::gltf::Document &doc,
  std::string model_name,
  const std::shared_ptr<SceneNode> &parent_node,
  const char *effect_name,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &inject_textures,
  bool use_env_light) {

  auto &en = Engine::Instance();

  bool effect_override = false;
  bool effect_srgb = false;
  if (effect_name != nullptr && strlen(effect_name) != 0) {
    if (strcmp(effect_name, "mineola:effect:pbr_srgb") == 0) {
      effect_srgb = true;
    } else if (strcmp(effect_name, "mineola:effect:pbr") != 0) {
      effect_override = true;
    }
  }

  // infer buffer view targets from various sources
  std::vector<BufferViewUsage> buffer_view_usages(doc.bufferViews.size());
  {
    // first directly from buffer views
    for (size_t idx = 0; idx < doc.bufferViews.size(); ++idx) {
      const auto &bv = doc.bufferViews[idx];
      buffer_view_usages[idx] = MapGLTFTargetType(bv.target);
    }

    // then from primitive indices
    for (const auto &m : doc.meshes) {
      for (const auto &p : m.primitives) {
        if (p.indices >= 0) {
          int32_t bv_id = doc.accessors[p.indices].bufferView;
          if (bv_id >= 0) {
            buffer_view_usages[bv_id] = BufferViewUsage::kIBO;
          }
        }
      }
    }

    // then images
    for (const auto &img : doc.images) {
      if (img.uri.empty() && img.bufferView >= 0) {  // buffer-based
        buffer_view_usages[img.bufferView] = BufferViewUsage::kTexture;
      }
    }

    // then animation
    for (const auto &anim : doc.animations) {
      for (const auto &sampler : anim.samplers) {
        int32_t accessor_in = sampler.input;
        int32_t accessor_out = sampler.output;
        if (accessor_in >= 0) {
          int32_t bv_id = doc.accessors[accessor_in].bufferView;
          if (bv_id >= 0) {
            buffer_view_usages[bv_id] = BufferViewUsage::kCPU;
          }
        }
        if (accessor_out >= 0) {
          int32_t bv_id = doc.accessors[accessor_out].bufferView;
          if (bv_id >= 0) {
            buffer_view_usages[bv_id] = BufferViewUsage::kCPU;
          }
        }
      }
    }

    // if still unknown, set as vbo
    std::replace(buffer_view_usages.begin(), buffer_view_usages.end(),
      BufferViewUsage::kUnknown, BufferViewUsage::kVBO);
  }

  // convert to buffer usages
  std::vector<std::unordered_set<uint32_t>> buffer_usages(doc.buffers.size());
  {
    for (size_t bv_id = 0; bv_id < doc.bufferViews.size(); ++bv_id) {
      int b_id = doc.bufferViews[bv_id].buffer;
      if (b_id >= 0 &&
        (buffer_view_usages[bv_id] == BufferViewUsage::kVBO ||
        buffer_view_usages[bv_id] == BufferViewUsage::kIBO)) {
        buffer_usages[b_id].insert((uint32_t)buffer_view_usages[bv_id]);
      }
    }
  }

  // load GPU buffers
  std::unordered_map<uint32_t, std::shared_ptr<GraphicsBuffer>> buffers;
  {
    // create graphics buffers
    for (size_t idx = 0; idx < doc.buffers.size(); ++idx) {
      const auto &b = doc.buffers[idx];
      std::vector<uint32_t> targets;

      const auto &usages = buffer_usages[idx];
      if (usages.size() == 0) {
        continue;
      }
      std::copy(usages.begin(), usages.end(), std::back_inserter(targets));

      auto buffer = std::make_shared<GraphicsBuffer>(
        GraphicsBuffer::STATIC, GraphicsBuffer::SEND, GraphicsBuffer::WRITE_ONLY, targets);
      buffer->Bind();
      buffer->SetData(b.byteLength, b.data.data());
      buffer->Unbind();
      buffers[(uint32_t)idx] = buffer;
    }
  }

  // load images
  std::unordered_map<int32_t, std::string> img_paths;
  std::unordered_map<int32_t, std::pair<const char*, uint32_t>> img_buffers;
  {
    for (size_t idx = 0; idx < doc.images.size(); ++idx) {
      const auto &img = doc.images[idx];
      if (img.uri.empty()) {  // buffer
        const auto &bv = doc.bufferViews[img.bufferView];
        const auto &b = doc.buffers[bv.buffer];
        img_buffers[(int32_t)idx] = std::make_pair((const char*)b.data.data() + bv.byteOffset, (uint32_t)bv.byteLength);
      } else if (img.IsEmbeddedResource()) {  // embedded
        // todo: support embedded URI data
        throw std::logic_error("Embedded image not implemented!");
      } else {
        img_paths[(int32_t)idx] = img.uri;
      }
    }
  }

  // infer texture srgb/rgb format from material usage
  std::unordered_set<uint32_t> srgb_textures;
  {
    for (const auto &m : doc.materials) {
      if (!m.emissiveTexture.empty()) {
        srgb_textures.insert((uint32_t)m.emissiveTexture.index);
      }
      if (!m.pbrMetallicRoughness.empty()) {
        const auto &m_pbr = m.pbrMetallicRoughness;
        if (!m_pbr.baseColorTexture.empty()) {
          srgb_textures.insert((uint32_t)m_pbr.baseColorTexture.index);
        }
      }
    }
  }

  // load textures
  std::unordered_map<uint32_t, std::string> texture_names;
  {
    for (size_t tex_idx = 0; tex_idx < doc.textures.size(); ++tex_idx) {
      const auto &t = doc.textures[tex_idx];
      if (t.source < 0) {
        continue;
      }
      bool srgb = srgb_textures.find(tex_idx) != srgb_textures.end();

      uint32_t min_filter = TextureDesc::kLinearMipmapLinear;
      uint32_t mag_filter = TextureDesc::kLinear;
      uint32_t wrap_s = TextureDesc::kRepeat;
      uint32_t wrap_t = TextureDesc::kRepeat;
      if (t.sampler >= 0 && doc.samplers.size() > t.sampler) {  // parse sampler modes
        const auto &s = doc.samplers[t.sampler];
        min_filter = MapGLTFMinFilter(s.minFilter);
        mag_filter = MapGLTFMagFilter(s.magFilter);
        wrap_s = MapGLTFWrapMode(s.wrapS);
        wrap_t = MapGLTFWrapMode(s.wrapT);
      }
      auto sampler_abbrev = AbbrevTextureMode(min_filter, mag_filter, wrap_s, wrap_t);

      if (img_paths.find(t.source) != img_paths.end()) {  // load from file
        std::string input_path = img_paths[t.source];
        std::string full_path;
        // find file on disk
        if (en.ResrcMgr().LocateFile(input_path.c_str(), full_path)) {
          auto texture_name = full_path + ":" + sampler_abbrev;
          if (!en.ResrcMgr().Find(texture_name)) {  // not loaded
            if (texture_helper::CreateTexture(
              texture_name.c_str(), full_path.c_str(), false, true, srgb,
              min_filter, mag_filter, wrap_s, wrap_t)) {
              texture_names[(uint32_t)tex_idx] = texture_name;
            } else {
              MLOG("Failed to create texture from file %s\n", full_path.c_str());
            }
          } else {  // already exists
            texture_names[(uint32_t)tex_idx] = texture_name;
          }
        }
      } else if (img_buffers.find(t.source) != img_buffers.end()) {  // load from buffer
        std::string texture_name = "tex:" + t.name
          + ":" + std::to_string(tex_idx)
          + ":" + sampler_abbrev;
        if (!en.ResrcMgr().Find(texture_name)) {  // not loaded
          if (texture_helper::CreateTexture(
            texture_name.c_str(), img_buffers[t.source].first, img_buffers[t.source].second,
            false, true, srgb,
            min_filter, mag_filter, wrap_s, wrap_t)) {
            texture_names[(uint32_t)tex_idx] = texture_name;
          } else {
            MLOG("Failed to create texture %s from memory!\n", texture_name.c_str());
          }
        } else {
          texture_names[(uint32_t)tex_idx] = texture_name;
        }
      }
    }
  }

  // load materials
  // <name, queue_id>
  using material_t = std::tuple<std::string, int16_t>;
  std::vector<material_t> materials;
  std::vector<MaterialFlags> materials_flags;
  {
    for (size_t mat_id = 0; mat_id < doc.materials.size(); ++mat_id) {
      const auto &m = doc.materials[mat_id];
      std::string name = "mat:" + model_name + ":" + std::to_string(mat_id);

      auto material = std::make_shared<Material>();
      MaterialFlags material_flags;

      int16_t q_id = Renderable::kQueueOpaque;
      if (m.alphaMode == fx::gltf::Material::AlphaMode::Blend) {
        q_id = Renderable::kQueueTransparent;
        material_flags.EnableBlending();
      } else if (m.alphaMode == fx::gltf::Material::AlphaMode::Mask) {
        material_flags.EnableAlphaCutOff();
        material->uniform_slots["alpha_threshold"] = uniform_helper::Wrap(m.alphaCutoff);
      }

      // additional textures
      if (!m.normalTexture.empty()) {
        material->texture_slots["normal_sampler"] =
          {texture_names[(uint32_t)m.normalTexture.index]};
        material_flags.EnableNormalMap((uint8_t)m.normalTexture.texCoord);
      }
      if (!m.occlusionTexture.empty()) {
        material->texture_slots["lightmap_sampler"] =
          {texture_names[(uint32_t)m.occlusionTexture.index]};
        material_flags.EnableOcclusionMap((uint8_t)m.occlusionTexture.texCoord);
      }
      if (!m.emissiveTexture.empty()) {
        material->texture_slots["emissive_sampler"] =
          {texture_names[(uint32_t)m.emissiveTexture.index]};
        material_flags.EnableEmissiveMap((uint8_t)m.emissiveTexture.texCoord);
      }
      material->emit = glm::vec3(
        m.emissiveFactor[0], m.emissiveFactor[1], m.emissiveFactor[2]);

      // pbr
      if (!m.pbrMetallicRoughness.empty()) {
        const auto &m_pbr = m.pbrMetallicRoughness;
        if (!m_pbr.baseColorTexture.empty()) {
          material->texture_slots["diffuse_sampler"] =
            {texture_names[(uint32_t)m_pbr.baseColorTexture.index]};
          material_flags.EnableDiffuseMap((uint8_t)m_pbr.baseColorTexture.texCoord);
        }
        if (!m_pbr.metallicRoughnessTexture.empty()) {
          material->texture_slots["metallic_roughness_sampler"] =
            {texture_names[(uint32_t)m_pbr.metallicRoughnessTexture.index]};
          material_flags.EnableMetallicRoughnessMap(
            (uint8_t)m_pbr.metallicRoughnessTexture.texCoord);
        }

        material->diffuse = glm::vec3(
          m_pbr.baseColorFactor[0], m_pbr.baseColorFactor[1], m_pbr.baseColorFactor[2]);
        material->alpha = m_pbr.baseColorFactor[3];
        material->specularity = m_pbr.metallicFactor;
        material->roughness = m_pbr.roughnessFactor;
      }

      // double side
      if (m.doubleSided) {
        material_flags.EnableDoubleSide();
      }

      // extensions
      if (!m.extensionsAndExtras.empty() && m.extensionsAndExtras.contains("extensions")) {
        auto &exts = m.extensionsAndExtras["extensions"];
        // KHR_materials_unlit
        if (exts.contains("KHR_materials_unlit")) {
          material_flags.SetUnlit();
        }
      }

      materials.push_back(std::make_tuple(name, q_id));
      materials_flags.push_back(material_flags);

      // add to resource manager
      en.ResrcMgr().Add(name, bd_cast<Resource>(material));
    }
  }

  // check which meshes contain skin
  std::unordered_set<uint32_t> skinned_mesh_ids;
  {
    for (const auto &n : doc.nodes) {
      if (n.skin >= 0) {
        skinned_mesh_ids.insert(n.mesh);
      }
    }
  }

  // load meshes
  std::vector<std::vector<std::shared_ptr<Renderable>>> meshes;
  {
    for (size_t mesh_idx = 0; mesh_idx < doc.meshes.size(); ++mesh_idx) {
      const auto &m = doc.meshes[mesh_idx];

      // create renderable
      std::vector<std::shared_ptr<Renderable>> mesh;

      // load all primitives
      // primitvie <-> renderable
      for (size_t p_idx = 0; p_idx < m.primitives.size(); ++p_idx) {
        auto renderable = std::make_shared<Renderable>();
        const auto &p = m.primitives[p_idx];

        AttribFlags attrib_flags;

        if (skinned_mesh_ids.find((uint32_t)mesh_idx) != skinned_mesh_ids.end()) {
          attrib_flags.EnableSkinning();
        }

        // vertex array holds all vertex streams
        auto va = std::make_shared<VertexArray>();

        // convert attributes to vertex streams and add to va
        for (const auto &attrib : p.attributes) {
          int semantics = MapGLTFSemantics(attrib.first);
          uint32_t accessor_id = attrib.second;
          int32_t buffer_view_id = doc.accessors[accessor_id].bufferView;
          if (buffer_view_id < 0) {
            continue;
          }
          int32_t buffer = doc.bufferViews[buffer_view_id].buffer;
          if (buffer < 0) {
            continue;
          }

          auto vs = std::make_shared<VertexStream>();
          const auto &accessor = doc.accessors[accessor_id];
          int comp_type = MapGLTFComponentType(accessor.componentType);
          int vec_length = MapGLTFVecLength(accessor.type);
          vs->layout.push_back({(uint32_t)semantics, (uint32_t)comp_type, (uint32_t)vec_length});
          vs->type = VST_VERTEX;
          vs->size = accessor.count;
          vs->offset = doc.bufferViews[buffer_view_id].byteOffset + accessor.byteOffset;
          vs->force_stride = doc.bufferViews[buffer_view_id].byteStride;
          vs->buffer_ptr = buffers[buffer];
          va->AddVertexStream(vs);

          SetAttribFlag(semantics, attrib_flags);
        }

        // add index array to va
        if (p.indices >= 0) {
          const auto &accessor = doc.accessors[p.indices];
          int32_t bv = accessor.bufferView;
          int comp_type = MapGLTFComponentType(accessor.componentType);
          auto is = std::make_shared<VertexStream>();
          is->layout.push_back({INDEX, (uint32_t)comp_type, 1});
          is->type = VST_INDEX;
          is->size = accessor.count;
          is->offset = doc.bufferViews[bv].byteOffset + accessor.byteOffset;
          is->force_stride = doc.bufferViews[bv].byteStride;
          is->buffer_ptr = buffers[doc.bufferViews[bv].buffer];
          va->SetIndexStream(is);
        }

        // set primitive type
        switch (p.mode) {
        case fx::gltf::Primitive::Mode::Points:
          va->PrimitiveType() = GL_POINTS;
          break;
        case fx::gltf::Primitive::Mode::Lines:
          va->PrimitiveType() = GL_LINES;
          break;
        case fx::gltf::Primitive::Mode::LineLoop:
          va->PrimitiveType() = GL_LINE_LOOP;
          break;
        case fx::gltf::Primitive::Mode::LineStrip:
          va->PrimitiveType() = GL_LINE_STRIP;
          break;
        case fx::gltf::Primitive::Mode::Triangles:
          va->PrimitiveType() = GL_TRIANGLES;
          break;
        case fx::gltf::Primitive::Mode::TriangleStrip:
          va->PrimitiveType() = GL_TRIANGLE_STRIP;
          break;
        case fx::gltf::Primitive::Mode::TriangleFan:
          va->PrimitiveType() = GL_TRIANGLE_FAN;
          break;
        default:
          va->PrimitiveType() = GL_POINTS;
          break;
        };

        // set material
        int32_t mat_id = p.material;
        if (mat_id >= 0) {
          renderable->AddVertexArray(va, std::get<0>(materials[mat_id]).c_str());
          renderable->SetQueueId(std::get<1>(materials[mat_id]));

          if (!effect_override) {
            // create or choose proper PBR shader
            const auto &mat_flags = materials_flags[mat_id];
            auto pbr_effect_name = SelectOrCreatePBREffect(
              effect_srgb, mat_flags, attrib_flags, use_env_light);
            if (pbr_effect_name.empty()) {
              pbr_effect_name = "mineola:effect:fallback";
            }
            renderable->SetEffect(pbr_effect_name.c_str());
          } else {  // effect override
            renderable->SetEffect(effect_name);
          }
        } else {
          renderable->AddVertexArray(va, "mineola:material:fallback");
          renderable->SetEffect("mineola:effect:fallback");
        }

        renderable->SetLayerMask(layer_mask);
        mesh.push_back(renderable);
      }

      meshes.push_back(std::move(mesh));
    }
  }

  // load node hierarchy
  std::vector<std::shared_ptr<SceneNode>> scene_nodes;
  std::vector<int32_t> scene_node_skins;
  {
    // create all SceneNode
    for (const auto &n : doc.nodes) {
      auto node = std::make_shared<SceneNode>();

      if (n.matrix != fx::gltf::defaults::IdentityMatrix) {
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat rotation;
        glm::mat4 mat(
          n.matrix[0], n.matrix[1], n.matrix[2], n.matrix[3],
          n.matrix[4], n.matrix[5], n.matrix[6], n.matrix[7],
          n.matrix[8], n.matrix[9], n.matrix[10], n.matrix[11],
          n.matrix[12], n.matrix[13], n.matrix[14], n.matrix[15]);
        if (!decompose(mat, scale, rotation, translation, skew, perspective)) {
          MLOG("Error: failed to decompose matrix for node %u!\n", (uint32_t)scene_nodes.size());
        } else {
          node->SetPosition(translation);
          node->SetRotation(rotation);
          node->SetScale(scale);
        }
      } else {
        node->SetPosition(glm::vec3(n.translation[0], n.translation[1], n.translation[2]));
        node->SetRotation(glm::quat(n.rotation[3], n.rotation[0], n.rotation[1], n.rotation[2]));
        node->SetScale(glm::vec3(n.scale[0], n.scale[1], n.scale[2]));
      }

      if (n.mesh >= 0) {
        std::copy(meshes[n.mesh].begin(), meshes[n.mesh].end(),
          std::back_inserter(node->Renderables()));
      }

      scene_nodes.push_back(node);
      scene_node_skins.push_back(n.skin);
    }

    // build tree structure
    std::vector<bool> is_children(scene_nodes.size());
    std::fill(is_children.begin(), is_children.end(), false);

    for (size_t idx = 0; idx < doc.nodes.size(); ++idx) {
      for (int32_t child : doc.nodes[idx].children) {
        SceneNode::LinkTo(scene_nodes[child], scene_nodes[idx]);
        is_children[child] = true;
      }
    }

    // link top level nodes to root
    for (size_t idx = 0; idx < is_children.size(); ++idx) {
      if (!is_children[idx]) {
        SceneNode::LinkTo(scene_nodes[idx], parent_node);
      }
    }
  }

  // load skins
  std::vector<std::shared_ptr<Skin>> skins;
  {
    for (const auto &s : doc.skins) {
      size_t num_joints = s.joints.size();
      if (num_joints == 0 || s.inverseBindMatrices < 0) {
        skins.push_back({});
        continue;
      }

      std::vector<std::weak_ptr<SceneNode>> nodes;
      std::vector<glm::mat4> inv_bind_mats;

      auto acc_id = s.inverseBindMatrices;
      auto flts = ParseNormalizedFloatBuffer(doc, acc_id);
      if (num_joints != flts.size() / 16) {
        MLOG("Error: wrong numbers of glTF skin inverse bind matrices!\n");
      }

      for (size_t i = 0; i < s.joints.size(); ++i) {
        auto joint_idx = s.joints[i];
        nodes.push_back(scene_nodes[joint_idx]);
        inv_bind_mats.push_back({
          flts[i * 16], flts[i * 16 + 1], flts[i * 16 + 2], flts[i * 16 + 3],
          flts[i * 16 + 4], flts[i * 16 + 5], flts[i * 16 + 6], flts[i * 16 + 7],
          flts[i * 16 + 8], flts[i * 16 + 9], flts[i * 16 + 10], flts[i * 16 + 11],
          flts[i * 16 + 12], flts[i * 16 + 13], flts[i * 16 + 14], flts[i * 16 + 15]
        });
      }

      auto skin = std::make_shared<Skin>();
      skin->SetJointNodes(std::move(nodes), std::move(inv_bind_mats));
      if (s.skeleton >= 0) {
        skin->SetRootNode(scene_nodes[s.skeleton]);
      }
      skins.push_back(std::move(skin));
    }
  }

  // associate skins to meshes
  {
    for (size_t nid = 0; nid < scene_nodes.size(); ++nid) {
      int32_t skin_id = scene_node_skins[nid];
      if (skin_id >= 0) {
        auto &node = scene_nodes[nid];
        node->Renderables()[0]->SetSkin(skins[skin_id]);
      }
    }
  }

  // load animations
  size_t num_animated_entities = 0;
  {
    for (const auto &anim : doc.animations) {

      animation::Animation animation;

      for (const auto &ch : anim.channels) {
        if (ch.target.node < 0 || ch.sampler < 0) {
          continue;
        }

        // bind to target
        animation::Channel channel;
        channel.target = scene_nodes[ch.target.node];

        // parse key frames
        const auto &s = anim.samplers[ch.sampler];
        switch (s.interpolation) {
          case fx::gltf::Animation::Sampler::Type::Step: {
            channel.interp = animation::Channel::kInterpStep;
            break;
          }
          case fx::gltf::Animation::Sampler::Type::Linear: {
            channel.interp = animation::Channel::kInterpLinear;
            break;
          }
          case fx::gltf::Animation::Sampler::Type::CubicSpline: {
            channel.interp = animation::Channel::kInterpCubicSpline;
            break;
          }
          default: {
            channel.interp = animation::Channel::kInterpLinear;
            break;
          }
        }

        // resample key frames
        ParseAnimationChannel(doc, ch, s.input, s.output, channel);

        // add to animation
        animation.AddChannel(std::move(channel));
      }  // each channel

      auto entity = std::make_shared<AnimatedEntity>();
      entity->AddAnimation(std::move(animation));

      std::string animation_name = anim.name;
      if (animation_name.empty()) {
        animation_name = "animation:" + std::to_string(num_animated_entities);
      }
      en.EntityMgr().Add(animation_name, bd_cast<Entity>(entity));
      num_animated_entities++;
    }  // each animation
  }

  return true;
}

}

namespace mineola { namespace gltf {

bool LoadScene(
  const char *fn,
  const std::shared_ptr<SceneNode> &parent_node,
  const char *effect_name,
  int layer_mask,
  const std::vector<std::pair<std::string, std::string>> &inject_textures,
  bool use_env_light) {
  if (fn == nullptr)
    return false;

  fx::gltf::Document gltf_doc;
  if (boost::algorithm::ends_with(fn, ".gltf")) {
    gltf_doc = fx::gltf::LoadFromText(fn);
  } else if (boost::algorithm::ends_with(fn, ".glb")) {
    gltf_doc = fx::gltf::LoadFromBinary(fn);
  } else {
    return false;
  }

  bool result = CreateSceneFromGLTFDoc(gltf_doc,
    fn, parent_node, effect_name, layer_mask, inject_textures, use_env_light);

  return result;
}

}} //end namespace
