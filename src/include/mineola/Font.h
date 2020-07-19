#ifndef MINEOLA_FONT_H
#define MINEOLA_FONT_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

namespace mineola {

class Font {
public:
  Font();
  virtual ~Font();

  virtual bool LoadFromFile(const char *fn) = 0;

  virtual void Layout(const std::string &text,
    std::vector<glm::vec2> &positions,  // unit: pixel
    std::vector<glm::vec2> &uvs) = 0;

  virtual std::string Name() = 0;
  virtual const std::string &MaterialName() = 0;
};

class BitmapFont : public Font {
public:
  BitmapFont();
  ~BitmapFont() override;

  bool LoadFromFile(const char *fn) override;

  void Layout(const std::string &text,
    std::vector<glm::vec2> &positions,
    std::vector<glm::vec2> &uvs) override;

  std::string Name() override;
  const std::string &MaterialName() override;

protected:
  struct CharDesc {
    int x{0};
    int y{0};
    int width{0};  // bbox width
    int height{0};  // bbox height
    int origin_x{0};  // shift wrt. baseline
    int origin_y{0};  // shift wrt. baseline
    int advance{0};
  };

  std::unordered_map<uint8_t, CharDesc> descs_;
  int img_width_{0};
  int img_height_{0};
  int size_{0};
  std::string name_;
  std::string filename_;
  std::string texture_name_;
  std::string material_name_;

  bool LoadJson(const char *fn);
  bool CreateMaterial();

};

} //namespace


#endif