#include "prefix.h"
#include <mineola/Font.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <mineola/TextureHelper.h>
#include <mineola/Engine.h>
#include <mineola/Material.h>
#include <mineola/UniformWrappers.h>

namespace mineola {

Font::Font() = default;

Font::~Font() = default;

BitmapFont::BitmapFont() = default;

BitmapFont::~BitmapFont() = default;

bool BitmapFont::LoadFromFile(const char *fn) {

  auto &en = Engine::Instance();
  std::string found_path;

  en.ResrcMgr().LocateFile(fn, found_path);
  if (!LoadJson(found_path.c_str())) {
    MLOG("Failed to parse font metadata\n");
    return false;
  }

  texture_name_ = std::string("tex:font:") + name_ + "_" + std::to_string(size_);
  material_name_ = std::string("mat:font:") + name_ + "_" + std::to_string(size_);

  en.ResrcMgr().LocateFile(filename_.c_str(), found_path);
  if (!mineola::texture_helper::CreateTexture(
    texture_name_.c_str(),
    found_path.c_str(),
    false,
    true)) {
    MLOG("Failed to load font image!\n");
    return false;
  }

  if (!CreateMaterial()) {
    MLOG("Failed to create font material %s!", material_name_.c_str());
    return false;
  }

  return true;
}

bool BitmapFont::LoadJson(const char *fn) {
  // load json
  using json = nlohmann::json;

  std::ifstream infile(fn);
  if (!infile.good()) {
    return false;
  }

  std::string file_contents {
    std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()
  };
  auto doc = json::parse(file_contents.c_str());
  infile.close();

  // parse json
  if (doc.find("name") == doc.end() || doc.find("size") == doc.end()) {
    MLOG("Font name/size not specified!\n");
    return false;
  }
  name_ = doc["name"].get<std::string>();
  size_ = doc["size"].get<int>();

  if (doc.find("filename") == doc.end()) {
    MLOG("Font image filename not specified!\n");
    return false;
  }
  filename_ = doc["filename"].get<std::string>();

  if (doc.find("width") == doc.end() || doc.find("height") == doc.end()) {
    MLOG("Font image size not specified!\n");
    return false;
  }

  img_width_ = doc["width"].get<int>();
  img_height_ = doc["height"].get<int>();

  if (doc.find("characters") == doc.end()) {
    MLOG("Font char table not found!\n");
    return false;
  }

  auto char_doc = doc["characters"];
  for (auto &[key, val] : char_doc.items()) {
    auto ascii = (uint8_t)key[0];
    CharDesc desc;
    desc.x = val["x"].get<int>();
    desc.y = val["y"].get<int>();
    desc.width = val["width"].get<int>();
    desc.height = val["height"].get<int>();
    desc.origin_x = val["originX"].get<int>();
    desc.origin_y = val["originY"].get<int>();
    desc.advance = val["advance"].get<int>();

    descs_[ascii] = std::move(desc);
  }

  return true;
}

bool BitmapFont::CreateMaterial() {
  auto material = std::make_shared<mineola::Material>();
  if (!material) {
    return false;
  }

  std::vector<std::string> texture_names;
  texture_names.push_back(texture_name_);
  material->texture_slots["font"] = {texture_names};

  auto &en = mineola::Engine::Instance();
  en.ResrcMgr().Add(material_name_, material);
  return true;
}

void BitmapFont::Layout(const std::string &text,
  std::vector<glm::vec2> &positions,
  std::vector<glm::vec2> &uvs) {

  auto str_len = text.size();

  float cursor = 0.f;
  for (size_t idx = 0; idx < str_len; ++idx) {
    auto glyph_idx = (uint8_t)text[idx];
    auto &desc = descs_[glyph_idx];

    float start_x = cursor - desc.origin_x;
    float start_y = desc.origin_y - desc.height;

    positions[idx * 4] = glm::vec2(start_x, start_y);
    positions[idx * 4 + 1] = glm::vec2(start_x + desc.width, start_y);
    positions[idx * 4 + 2] = glm::vec2(start_x + desc.width, start_y + desc.height);
    positions[idx * 4 + 3] = glm::vec2(start_x, start_y + desc.height);

    float start_u = (float)desc.x / img_width_;
    float start_v = (float)desc.y / img_height_;
    float width_u = (float)desc.width / img_width_;
    float height_v = (float)desc.height / img_height_;

    uvs[idx * 4] = glm::vec2(start_u, start_v + height_v);
    uvs[idx * 4 + 1] = glm::vec2(start_u + width_u, start_v + height_v);
    uvs[idx * 4 + 2] = glm::vec2(start_u + width_u, start_v);
    uvs[idx * 4 + 3] = glm::vec2(start_u, start_v);

    cursor += desc.advance;
  }

}

const std::string &BitmapFont::MaterialName() {
  return material_name_;
}

std::string BitmapFont::Name() {
  return name_ + "_" + std::to_string(size_);
}


}