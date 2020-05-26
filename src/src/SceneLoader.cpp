#include "prefix.h"
#include <mineola/SceneLoader.h>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <nlohmann/json.hpp>
#include <mineola/glutility.h>
#include <mineola/SceneNode.h>
#include <mineola/Engine.h>
#include <mineola/Light.h>
#include <mineola/Camera.h>
#include <mineola/PrimitiveHelper.h>
#include <mineola/Renderable.h>
#include <mineola/Material.h>
#include <mineola/GLMHelper.h>
#include <mineola/GLEffect.h>
#include <mineola/RenderStateFactory.h>
#include <mineola/UniformWrappers.h>
#include <mineola/Texture.h>
#include <mineola/TextureHelper.h>
#include <mineola/PixelType.h>
#include <mineola/Framebuffer.h>
#include <mineola/FileSystem.h>

namespace {
template <typename Op, typename ...Args>
bool RunSequential(const std::vector<Op> &ops, Args &&...args) {
  for (size_t i = 0; i < ops.size(); ++i) {
    if (ops[i](std::forward<Args>(args)...))
      return true;
  }
  return false;
}

template<typename T>
T JArray2Vec(const nlohmann::json &j) {
  T result;
  if (j.is_array()) {
    if (result.length() != (int)j.size()) {
      throw std::length_error("glm::tvec and json array size mismatch!");
    }
    for (int i = 0; i < result.length(); ++i) {
      result[i] = j[i].get<float>();
    }
  }
  return result;
}

}  // namespace

namespace mineola {

bool BuildSceneFromConfigStream(std::istream &ins,
  const std::vector<GeometryLoaderT> &geometry_loaders) {

  std::string file_contents {
    std::istreambuf_iterator<char>(ins), std::istreambuf_iterator<char>()
  };

  return BuildSceneFromConfig(file_contents.c_str(), geometry_loaders);
}


bool BuildSceneFromConfigFile(const char *filename,
  const std::vector<GeometryLoaderT> &geometry_loaders) {

  std::string found_fn;
  auto &en = Engine::Instance();

  if (!en.ResrcMgr().LocateFile(filename, found_fn)) {
    MLOG("Failed to locate file %s\n", filename);
    return false;
  }

  std::ifstream infile(found_fn.c_str());
  std::string file_contents {
    std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()
  };
  infile.close();

  bool result = BuildSceneFromConfig(file_contents.c_str(), geometry_loaders);
  return result;
}

bool BuildSceneFromConfig(const char *config_str,
  const std::vector<GeometryLoaderT> &geometry_loaders) {

  using json = nlohmann::json;

  auto &en = Engine::Instance();
  auto doc = json::parse(config_str);

  // SceneNode tree
  std::unordered_map<std::string, std::shared_ptr<SceneNode>> nodes_dict;
  if (doc.find("nodes") != doc.end()) {
    const auto &nodes = doc["nodes"];
    for (const auto &node_desc : nodes) {
      // parse info
      std::string name = node_desc["name"].get<std::string>();
      std::shared_ptr<SceneNode> new_node(std::make_shared<SceneNode>());

      if (node_desc.find("eye") != node_desc.end()) {  // lookat rbt
        glm::vec3 eye(0.0f, 0.0f, -1.0f), target, up_dir(0.0f, 1.0f, 0.0f);
        eye = ParseVec3(node_desc["eye"].get<std::string>());
        if (node_desc.find("target") != node_desc.end())
          target = ParseVec3(node_desc["target"].get<std::string>());
        if (node_desc.find("up") != node_desc.end())
          up_dir = ParseVec3(node_desc["up"].get<std::string>());
        new_node->SetRbt(math::Rbt::LookAt(eye, target, up_dir));

        if (node_desc.find("scale") != node_desc.end()) {
          glm::vec3 scale = ParseVec3(node_desc["scale"].get<std::string>());
          new_node->SetScale(scale);
        }
      } else {  // TRS rbt
        glm::vec3 translate(0.f, 0.f, 0.f);
        if (node_desc.find("translate") != node_desc.end())
          translate = ParseVec3(node_desc["translate"].get<std::string>());
        glm::quat rotation(1.f, 0.f, 0.f, 0.f);
        if (node_desc.find("rotate") != node_desc.end()) {
          glm::vec4 rotate_v = ParseVec4(node_desc["rotate"].get<std::string>());
          rotation = (glm::quat)rotate_v;
        }
        glm::vec3 scale(1.f, 1.f, 1.f);
        if (node_desc.find("scale") != node_desc.end())
          scale = ParseVec3(node_desc["scale"].get<std::string>());

        if (node_desc.find("matrix") != node_desc.end()) {
          auto &matrix = node_desc["matrix"];
          if (matrix.find("raw") != matrix.end()) {
            auto mat = ParseMat4(matrix["raw"].get<std::string>());
            if (matrix.find("coordsys") != matrix.end()) {
              auto coordsys = matrix["coordsys"].get<std::string>();
              if (strcmp(coordsys.c_str(), "mitsuba") == 0)
                mat = mat * glm::mat4(-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            }
            glm::vec3 skew; glm::vec4 perspective;
            // skew and perspective components are abandoned
            if (!glm::decompose(mat, scale, rotation, translate, skew, perspective))
              MLOG("Node %s matrix cannot be decomposed!\n", name.c_str());
          }
        }

        // create node
        new_node->SetPosition(translate);
        new_node->SetRotation(rotation);
        new_node->SetScale(scale);
      }
      new_node->SetName(name);

      // add to dict
      nodes_dict[name] = new_node;

      // link to parent
      if (node_desc.find("parent") != node_desc.end()) {
        std::string parent_name = node_desc["parent"].get<std::string>();
        if (parent_name == "root") {
          SceneNode::LinkTo(new_node, en.Scene());
        } else {
          auto iter = nodes_dict.find(parent_name);
          if (iter != nodes_dict.end()) {
            SceneNode::LinkTo(new_node, iter->second);
          }
        }
      }
    }
  }

  // attach renderables
  if (doc.find("geometries") != doc.end()) {
    const auto &geos = doc["geometries"];
    for (const auto &geo : geos) {
      std::string effect = "mineola:effect:fallback";
      if (geo.find("effect") != geo.end()) {
        effect = geo["effect"].get<std::string>();
      }

      int layer = 0;
      if (geo.find("layer") != geo.end()) {
        const auto &layer_node = geo["layer"];
        if (layer_node.is_array()) {
          for (int layer_idx : layer_node) {
            layer |= 1 << layer_idx;
          }
        } else {
          layer = 1 << layer_node.get<int>();
        }
      } else {
        layer = RenderPass::RENDER_LAYER_0;
      }

      std::shared_ptr<SceneNode> node = en.Scene();
      if (geo.find("node") != geo.end()) {
        std::string node_name = geo["node"].get<std::string>();
        auto iter = nodes_dict.find(node_name);
        if (iter != nodes_dict.end()) {
          node = iter->second;
        } else {
          auto found_node = SceneNode::FindNodeByName(node_name.c_str(), en.Scene().get());
          if (found_node) {
            node = found_node;
          } else {
            MLOG("Node %s not found!\n", node_name.c_str());
          }
        }
      }

      if (geo.find("primitive") != geo.end()) {
        auto va = std::make_shared<vertex_type::VertexArray>();
        std::string primitive = geo["primitive"].get<std::string>();
        if (primitive == "rect")
          primitive_helper::BuildRect(2.f, *va);
        else if (primitive == "rect_xy")
          primitive_helper::BuildRectXY(2.f, *va);
        else if (primitive == "sphere")
          primitive_helper::BuildSphere(3, *va);
        else if (primitive == "cube")
          primitive_helper::BuildCube(2.f, *va);

        std::string material = "mineola:material:fallback";
        if (geo.find("material") != geo.end()) {
          material = geo["material"].get<std::string>();
          auto mat = bd_cast<Material>(en.ResrcMgr().Find(material.c_str()));
          if (!mat) {
            mat = std::make_shared<Material>();
            mat->alpha = 1.0f;
            mat->specularity = 30.0f;
            mat->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
            mat->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
            mat->specular = glm::vec3(0.2f, 0.2f, 0.2f);
            mat->emit = glm::vec3(0.f, 0.f, 0.f);

            if (geo.find("shadowmap") != geo.end() && geo["shadowmap"].get<bool>())
              mat->texture_slots["shadowmap"] = {"mineola:rt:shadowmap:depth_texture"};
            en.ResrcMgr().Add(material, mat);
          }
        }

        auto renderable = std::make_shared<Renderable>();
        renderable->AddVertexArray(va, material.c_str());
        renderable->SetEffect(effect.c_str());
        renderable->SetLayerMask(layer);
        node->Renderables().push_back(renderable);
      }
      else {
        std::string filename = geo["filename"].get<std::string>();
        std::string found_fn;
        if (en.ResrcMgr().LocateFile(filename.c_str(), found_fn)) {
          std::vector<std::pair<std::string, std::string>> inject_textures;
          if (geo.find("shadowmap") != geo.end() && geo["shadowmap"].get<bool>()) {
            inject_textures.push_back({"shadowmap", "mineola:rt:shadowmap:depth_texture"});
          }

          // add file directory to search paths
          std::string input_path, input_fn;
          std::tie(input_path, input_fn) = file_system::SplitPath(found_fn);
          en.ResrcMgr().AddSearchPath(input_path.c_str());

          if (!RunSequential(geometry_loaders, found_fn.c_str(), node,
            effect.c_str(), layer, inject_textures)) {
            MLOG("Geometry %s not loaded!\n", filename.c_str());
          }
          en.ResrcMgr().PopSearchPath(input_path.c_str());
        } else {
          MLOG("Geometry file %s not found!\n", filename.c_str());
        }
      }
    }
  }

  // attach lights
  if (doc.find("lights") != doc.end()) {
    const auto &lights = doc["lights"];
    for (const auto &light_config : lights) {
      size_t index = light_config["index"].get<int>();
      std::string proj_type = light_config["proj"].get<std::string>();
      std::shared_ptr<Light> light(new Light(index));
      if (proj_type == "perspective") {
        float fovy = light_config["fovy"].get<float>();
        float aspect = light_config["aspect"].get<float>();
        float z_near = light_config["near"].get<float>();
        float z_far = light_config["far"].get<float>();
        light->SetProjParams(glm::radians(fovy), aspect, z_near, z_far);
      } else if (proj_type == "orthographic") {
        float left = light_config["left"].get<float>();
        float right = light_config["right"].get<float>();
        float bottom = light_config["bottom"].get<float>();
        float top = light_config["top"].get<float>();
        float z_near = light_config["near"].get<float>();
        float z_far = light_config["far"].get<float>();
        light->SetOrthoProjParams(left, right, bottom, top, z_near, z_far);
      }

      if (!light) {  // if no light was created
        MLOG("Light params invalid!\n");
        continue;
      }

      if (light_config.find("intensity") != light_config.end()) {
        glm::vec3 intensity = ParseVec3(light_config["intensity"].get<std::string>());
        light->SetIntensity(intensity);
      }

      if (light_config.find("node") != light_config.end()) {
        auto node_name = light_config["node"].get<std::string>();
        auto iter = nodes_dict.find(node_name);
        if (iter != nodes_dict.end()) {
          iter->second->Lights().push_back(light);
        } else {
          auto found_node = SceneNode::FindNodeByName(node_name.c_str(), en.Scene().get());
          if (found_node) {
            found_node->Lights().push_back(light);
          } else {
            MLOG("Node %s not found!\n", node_name.c_str());
          }
        }
      } else {
        en.Scene()->Lights().push_back(light);
      }
    }
  }

  // attach cameras
  if (doc.find("cameras") != doc.end()) {
    const auto &cameras = doc["cameras"];
    for (const auto &camera_config : cameras) {
      std::string name = camera_config["name"].get<std::string>();
      std::string proj_type = camera_config["proj"].get<std::string>();
      std::shared_ptr<Camera> camera;
      if (proj_type == "perspective") {
        camera.reset(new Camera(true));
        float fovy = camera_config["fovy"].get<float>();
        float z_near = camera_config["near"].get<float>();
        float z_far = camera_config["far"].get<float>();
        camera->SetProjParams(glm::radians(fovy), z_near, z_far);
      } else if (proj_type == "orthographic") {
        camera.reset(new Camera(false));
        float left = camera_config["left"].get<float>();
        float right = camera_config["right"].get<float>();
        float bottom = camera_config["bottom"].get<float>();
        float top = camera_config["top"].get<float>();
        float z_near = camera_config["near"].get<float>();
        float z_far = camera_config["far"].get<float>();
        camera->SetOrthoProjParams(left, right, bottom, top, z_near, z_far);
      }

      if (!camera) {
        MLOG("Invalid camera params!\n");
        continue;
      }

      en.CameraMgr().Add(name, camera);

      if (camera_config.find("node") != camera_config.end()) {
        auto node_name = camera_config["node"].get<std::string>();
        auto iter = nodes_dict.find(node_name);
        if (iter != nodes_dict.end()) {
          iter->second->Cameras().push_back(camera);
        } else {
          auto found_node = SceneNode::FindNodeByName(node_name.c_str(), en.Scene().get());
          if (found_node) {
            found_node->Cameras().push_back(camera);
          } else {
            MLOG("Node %s not found!\n", node_name.c_str());
          }
        }
      } else {
        en.Scene()->Cameras().push_back(camera);
      }
    }
  }

  // create effects
  if (doc.find("effects") != doc.end()) {
    const auto &effects = doc["effects"];
    for (const auto &effect : effects) {
      if (effect.find("name") == effect.end()) {
        MLOG("Anonymous effect not support\n");
      }
      std::string effect_name = effect["name"].get<std::string>();
      if (effect.find("vertex") == effect.end()) {
        MLOG("Missing vertex shader of %s!\n", effect_name.c_str());
      }
      std::string vertex_shader = effect["vertex"].get<std::string>();
      if (effect.find("fragment") == effect.end()) {
        MLOG("Missing fragment shader of %s\n", effect_name.c_str());
      }
      std::string fragment_shader = effect["fragment"].get<std::string>();
      std::vector<std::unique_ptr<RenderState>> render_states;
      if (effect.find("states") != effect.end()) {
        for (auto &el : effect["states"].items()) {
          std::unique_ptr<RenderState> state =
            mineola::RenderStateFactory::CreateRenderState(
              el.key(), el.value().get<std::string>());
          if (state) {
            render_states.push_back(std::move(state));
          }
        }
      }
      CreateEffectFromFileHelper(
        effect_name.c_str(),
        vertex_shader.c_str(),
        fragment_shader.c_str(),
        nullptr, std::move(render_states));
    }
  }

  // create textures
  if (doc.find("textures") != doc.end()) {
    for (const auto &texture : doc["textures"]) {
      if (texture.find("name") == texture.end()) {
        MLOG("Anonymous texture not supported!\n");
      }
      auto texture_name = texture["name"].get<std::string>();
      bool is_srgb = true;
      if (texture.find("srgb") != texture.end()) {
        is_srgb = texture["srgb"].get<bool>();
      }

      if (texture.find("filename") != texture.end()) {
        // a texture from file
        auto filename = texture["filename"].get<std::string>();
        bool mipmap = false;
        if (texture.find("mipmap") != texture.end()) {
          mipmap = texture["mipmap"].get<bool>();
        }
        texture_helper::CreateTexture(texture_name.c_str(), filename.c_str(), mipmap, is_srgb);
      } else {
        // empty texture
        uint32_t type = GL_TEXTURE_2D, width = 1, height = 1, depth = 1;
        bool dimension_specified = false;
        if (texture.find("dimensions") != texture.end()) {
          auto &dimensions = texture["dimensions"];
          if (dimensions.is_string()) {
            // this texture's dimension resemble that of another texture
            auto texture = bd_cast<Texture>(en.ResrcMgr().Find(dimensions.get<std::string>()));
            if (texture) {
              auto &desc = texture->Desc();
              type = desc.type;
              width = desc.width;
              height = desc.height;
              depth = desc.depth;
              dimension_specified = true;
            }
          } else if (dimensions.is_array()) {
            auto &whd = dimensions;
            dimension_specified = true;
            switch (whd.size()) {
            case 2:
              type = GL_TEXTURE_2D;
              width = whd[0].get<uint32_t>();
              height = whd[1].get<uint32_t>();
              break;
            case 3:
              type = GL_TEXTURE_3D;
              width = whd[0].get<uint32_t>();
              height = whd[1].get<uint32_t>();
              depth = whd[2].get<uint32_t>();
              break;
            default:
              dimension_specified = false;
            }
          }
        }
        if (!dimension_specified) {
          MLOG("Error: texture %s dimension not specified!\n", texture_name.c_str());
          continue;
        }

        uint32_t array_size = 1;
        if (texture.find("array_size") != texture.end()) {
          array_size = texture["array_size"].get<uint32_t>();
        }
        uint32_t levels = 1;
        if (texture.find("levels") != texture.end()) {
          levels = texture["levels"].get<uint32_t>();
        }
        uint32_t samples = 1;
        if (texture.find("samples") != texture.end()) {
          samples = texture["samples"].get<uint32_t>();
        }

        bool is_depth = false;
        uint32_t depth_bits = 24;
        if (texture.find("depth_bits") != texture.end()) {
          is_depth = true;
          depth_bits = texture["depth_bits"].get<uint32_t>();
        }

        if (is_depth) {
          bool stencil = false;
          if (texture.find("stencil") != texture.end()) {
            stencil = texture["stencil"].get<bool>();
          }
          bool depth_compare = false;
          if (texture.find("depth_compare") != texture.end()) {
            depth_compare = texture["depth_compare"].get<bool>();
          }
          texture_helper::CreateDepthTexture(texture_name.c_str(),
            width, height, depth_bits, stencil,
            samples, depth_compare);
        } else {
          uint8_t bpc = 8;
          if (texture.find("bpc") != texture.end()) {
            bpc = (uint8_t)texture["bpc"].get<uint32_t>();
          }
          uint32_t channel = 4;
          if (texture.find("channel") != texture.end()) {
            channel = texture["channel"].get<uint32_t>();
          }
          bool is_signed = false, is_float = false;
          if (texture.find("signed") != texture.end()) {
            is_signed = texture["signed"].get<bool>();
          }
          if (texture.find("float") != texture.end()) {
            is_float = texture["float"].get<bool>();
          }
          uint32_t format = 0, internal_format = 0, data_type = 0;
          pixel_type::Map2GL(bpc, channel, is_signed, is_float, is_srgb,
            internal_format, format, data_type);
          texture_helper::CreateEmptyTexture(texture_name.c_str(), type,
            width, height, depth, array_size, levels,
            format, internal_format, data_type, samples);
        }
      }
    }
  }

  // create materials
  if (doc.find("materials") != doc.end()) {
    for (auto &m : doc["materials"]) {
      if (m.find("name") == m.end()) {
        MLOG("Anonymous material not supported!\n");
      }

      auto material_name = m["name"].get<std::string>();
      auto material = std::make_shared<Material>();

      if (m.find("uniforms") != m.end()) {
        for (auto &uniform : m["uniforms"]) {
          if (uniform.find("name") == uniform.end() || uniform.find("type") == uniform.end()) {
            MLOG("Uniform name and type must be specified!\n");
          }
          auto uniform_name = uniform["name"].get<std::string>();
          std::string uniform_type = uniform["type"].get<std::string>();

          if (uniform_type == "float") {
            float value = uniform["value"].get<float>();
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(value);
          } else if (uniform_type == "float[]") {
            std::vector<float> value;
            for (const auto &v : uniform["value"]) {
              value.push_back(v.get<float>());
            }
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(std::move(value));
          } else if (uniform_type == "vec2") {
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(JArray2Vec<glm::vec2>(uniform["value"]));
          } else if (uniform_type == "vec2[]") {
            std::vector<glm::vec2> value;
            for (const auto &v : uniform["value"]) {
              value.push_back(JArray2Vec<glm::vec4>(v));
            }
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(std::move(value));
          } else if (uniform_type == "vec3") {
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(
              JArray2Vec<glm::vec3>(uniform["value"]));
          } else if (uniform_type == "vec3[]") {
            std::vector<glm::vec3> value;
            for (const auto &v : uniform["value"]) {
              value.push_back(JArray2Vec<glm::vec4>(v));
            }
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(std::move(value));
          } else if (uniform_type == "vec4") {
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(
              JArray2Vec<glm::vec4>(uniform["value"]));
          } else if (uniform_type == "vec4[]") {
            std::vector<glm::vec4> value;
            for (const auto &v : uniform["value"]) {
              value.push_back(JArray2Vec<glm::vec4>(v));
            }
            material->uniform_slots[uniform_name] = uniform_helper::Wrap(std::move(value));
          } else if (uniform_type == "texture") {
            std::vector<std::string> texture_names;
            for (const auto &v : uniform["texture_names"]) {
              texture_names.push_back(v.get<std::string>());
            }
            material->texture_slots[uniform_name] = std::move(texture_names);
          } else {
            MLOG("Unknown uniform type %s!\n", uniform_type.c_str());
          }
        }
      }
      en.ResrcMgr().Add(material_name, material);
    }
  }

  // create render targets
  if (doc.find("render_targets") != doc.end()) {
    for (auto &rt : doc["render_targets"]) {
      if (rt.find("name") == rt.end()) {
        MLOG("Anonymous render target not supported!\n");
      }
      auto rt_name = rt["name"].get<std::string>();

      size_t num_pbos = 1;
      if (rt.find("num_pbos") != rt.end()) {
        num_pbos = rt["num_pbos"].get<uint32_t>();
      }
      auto fb = std::make_shared<InternalFramebuffer>(num_pbos);

      if (rt.find("depth_texture") != rt.end()) {
        fb->AttachTexture(Framebuffer::AT_DEPTH, rt["depth_texture"].get<std::string>().c_str());
      }

      if (rt.find("color_texture") != rt.end()) {
        fb->AttachTexture(Framebuffer::AT_COLOR0, rt["color_texture"].get<std::string>().c_str());
      } else if (rt.find("color_textures") != rt.end()) {
        uint32_t idx = 0;
        for (const auto &tex_desc : rt["color_textures"]) {
          fb->AttachTexture(Framebuffer::AT_COLOR0 + idx, tex_desc.get<std::string>().c_str());
        }
      }

      auto status = fb->CheckStatus();
      const char *s = getFramebufferStatusString(status);
      if (s) {
        MLOG("Error: %s!\n", s);
      } else {
        en.ResrcMgr().Add(rt_name, fb);
      }
    }
  }

  // create render passes
  if (doc.find("passes") != doc.end()) {
    for (const auto &pass : doc["passes"]) {
      RenderPass render_pass;

      render_pass.layer_mask = RenderPass::RENDER_LAYER_ALL;
      if (pass.find("layer") != pass.end()) {
        auto &layer = pass["layer"];
        if (layer.is_array()) {
          render_pass.layer_mask = 0;
          for (const auto &l : layer) {
            render_pass.layer_mask |= 1 << l.get<uint32_t>();
          }
        } else {
          render_pass.layer_mask = 1 << layer.get<uint32_t>();
        }
      }

      render_pass.sfx = RenderPass::SFX_PASS_NONE;
      if (pass.find("sfx") != pass.end()) {
        std::string sfx_str = pass["sfx"].get<std::string>();
        if (sfx_str == "depth") {
          render_pass.sfx = RenderPass::SFX_PASS_DEPTH;
        } else if (sfx_str == "shadowmap") {
          render_pass.sfx = RenderPass::SFX_PASS_SHADOWMAP;
        } else if (sfx_str == "hdr") {
          render_pass.sfx = RenderPass::SFX_PASS_HDR;
        }
      }

      render_pass.clear_flag = RenderPass::CLEAR_ALL;
      if (pass.find("clear") != pass.end()) {
        render_pass.clear_flag = RenderPass::CLEAR_NONE;
        std::string clear_str = pass["clear"].get<std::string>();
        if (clear_str.find("color") != std::string::npos) {
          render_pass.clear_flag |= RenderPass::CLEAR_COLOR;
        }
        if (clear_str.find("depth") != std::string::npos) {
          render_pass.clear_flag |= RenderPass::CLEAR_DEPTH;
        }
      }

      render_pass.override_effect = "";
      if (pass.find("effect") != pass.end()) {
        render_pass.override_effect = pass["effect"].get<std::string>();
      }
      render_pass.override_render_target = "";
      if (pass.find("target") != pass.end()) {
        render_pass.override_render_target = pass["target"].get<std::string>();
      }
      render_pass.override_camera = "";
      if (pass.find("camera") != pass.end()) {
        render_pass.override_camera = pass["camera"].get<std::string>();
      }
      render_pass.override_material = "";
      if (pass.find("material") != pass.end()) {
        render_pass.override_material = pass["material"].get<std::string>();
      }

      en.RenderPasses().push_back(render_pass);
    }
  }

  return true;
}

}
