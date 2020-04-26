#include "prefix.h"
#include "../include/PolygonSoupSerialization.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "../include/Engine.h"

namespace mineola {

  bool WriteSoupToPLY(const char *fn, const PolygonSoup &soup) {
    std::ofstream out(fn, std::ios_base::out);
    if (!out.good()) return false;

    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "comment FileFormat PolygonSoup\n";
    out << "comment Mineola generated\n";
    if (!soup.texture_filename.empty())
      out << "comment TextureFile " << soup.texture_filename << "\n";
    out << "element vertex " << soup.vertices.size() << "\n";
    out << "property float x\nproperty float y\nproperty float z\n";
    if (soup.has_vertex_normal)
      out << "property float nx\nproperty float ny\nproperty float nz\n";
    if (soup.has_vertex_texcoord)
      out << "property float u\nproperty float v\n";
    if (soup.has_vertex_color)
      out << "property uint8 red\nproperty uint8 green\nproperty uint8 blue\n";
    out << "element face " << soup.faces.size() << "\n";
    out << "property list uchar int vertex_indices\n";
    if (soup.has_face_texcoord)
      out << "property list uchar float texcoord\n";
    out << "end_header\n";

    for (const auto &v : soup.vertices) {
      out << v.pos.x << " " << v.pos.y << " " << v.pos.z << " ";
      if (soup.has_vertex_normal)
        out << v.normal.x << " " << v.normal.y << " " << v.normal.z << " ";
      if (soup.has_vertex_texcoord)
        out << v.tex.x << " " << v.tex.y << " ";
      if (soup.has_vertex_color) {
        out << (uint32_t)v.color.rgba[0] << " " << (uint32_t)v.color.rgba[1] << " "
        << (uint32_t)v.color.rgba[2] << " ";
      }
      out << "\n";
    }

    for (const auto &f : soup.faces) {
      out << f.indices.size() << " ";
      for (auto index : f.indices)
        out << index << " ";
      if (soup.has_face_texcoord) {
        out << 2 * f.texcoords.size() << " ";
        for (const auto &tex : f.texcoords)
          out << tex.x << " " << tex.y << " ";
      }
      out << "\n";
    }
    out.close();
    return true;
  }

  bool LoadSoupFromPLY(const char *fn, PolygonSoup &soup) {
    std::string found_fn;
    if (!Engine::Instance().ResrcMgr().LocateFile(fn, found_fn))
      return false;
    std::ifstream infile(found_fn, std::ios::binary);
    if (!infile.good()) return false;

    bool result = LoadSoupFromPLY(infile, soup);
    infile.close();
    return result;
  }

  bool LoadSoupFromPLY(std::istream &ins, PolygonSoup &soup) {

    soup.vertices.clear();
    soup.faces.clear();

    std::string line;
    std::getline(ins, line); // "ply"
    if (line != "ply") {
      return false;
    }

    std::getline(ins, line); // "format ascii/binary 1.0"
    // check binary or ascii
    bool is_ascii = true;
    bool is_little_endian = true;
    {
      std::vector<std::string> str_vec;
      boost::algorithm::split(str_vec, line, boost::algorithm::is_any_of(" "));
      if (str_vec.size() >= 2) {
        if (str_vec[1] == "binary_little_endian") {
          is_ascii = false;
        } else if (str_vec[1] == "binary_big_endian") {
          is_ascii = false;
          is_little_endian = false;
        } else {
          is_ascii = true;
        }
      }
    }

    soup.has_vertex_normal = soup.has_vertex_texcoord = soup.has_face_texcoord = false;
    std::vector<std::string> str_vec;
    while (ins.good()) { // header
      std::getline(ins, line);
      boost::algorithm::split(str_vec, line, boost::algorithm::is_any_of(" "));
      std::for_each(str_vec.begin(), str_vec.end(), [](std::string &str) {
        str = boost::trim_copy(str);
      });
      if (boost::algorithm::trim_copy(line) == "end_header") {
        break;
      } else if (str_vec[0] == "comment") {
        if (str_vec.size() > 2) {
          if (str_vec[1] == "TextureFile") {
            soup.texture_filename = boost::algorithm::trim_copy(str_vec[2]);
          }
        }
      } else if (str_vec[0] == "element") {
        if (str_vec.size() > 2) {
          if (str_vec[1] == "vertex" ) {
            try {
              soup.vertices.resize((size_t)std::stoi(str_vec[2]));
            } catch (const std::exception &e) {
              MLOG("%s\n", e.what());
              return false;
            }
          } else if (str_vec[1] == "face") {
            try {
              soup.faces.resize((size_t)std::stoi(str_vec[2]));
            } catch (const std::exception &e) {
              MLOG("%s\n", e.what());
              return false;
            }
          }
        }
      } else if (str_vec[0] == "property") {
        if (str_vec.back() == "nz") {
          soup.has_vertex_normal = true;
        } else if (str_vec.back() == "v") {
          soup.has_vertex_texcoord = true;
        } else if (str_vec.back() == "texcoord") {
          soup.has_face_texcoord = true;
        } else if (str_vec.back() == "red"
          || str_vec.back() == "green" || str_vec.back() == "blue") {
          soup.has_vertex_color = true;
        }
      }
    }

    std::vector<char> read_buffer;
    read_buffer.resize(sizeof(float) * 3);
    for (auto &vertex : soup.vertices) {
      if (is_ascii) {
        std::getline(ins, line);
        boost::algorithm::split(str_vec, line, boost::algorithm::is_any_of(" "));
        size_t total_size = 3 +
          (soup.has_vertex_normal ? 3 : 0) +
          (soup.has_vertex_texcoord ? 2 : 0) +
          (soup.has_vertex_color ? 3 : 0);
        if (str_vec.size() < total_size)
          return false;
        size_t i = 0;
        try {
          vertex.pos.x = std::stof(str_vec[i++]);
          vertex.pos.y = std::stof(str_vec[i++]);
          vertex.pos.z = std::stof(str_vec[i++]);
          if (soup.has_vertex_normal) {
            vertex.normal.x = std::stof(str_vec[i++]);
            vertex.normal.y = std::stof(str_vec[i++]);
            vertex.normal.z = std::stof(str_vec[i++]);
          }
          if (soup.has_vertex_texcoord) {
            vertex.tex.x = std::stof(str_vec[i++]);
            vertex.tex.y = std::stof(str_vec[i++]);
          }
          if (soup.has_vertex_color) {
            vertex.color.rgba[0] = (uint8_t)std::stoi(str_vec[i++]);
            vertex.color.rgba[1] = (uint8_t)std::stoi(str_vec[i++]);
            vertex.color.rgba[2] = (uint8_t)std::stoi(str_vec[i++]);
            vertex.color.rgba[3] = 0;
          }
        } catch (const std::exception &e) {
          MLOG("%s\n", e.what());
          return false;
        }
      } else if (is_little_endian) {
        ins.read(&read_buffer[0], sizeof(float) * 3);
        vertex.pos.x = *(float*)(&read_buffer[0]);
        vertex.pos.y = *(float*)(&read_buffer[sizeof(float)]);
        vertex.pos.z = *(float*)(&read_buffer[sizeof(float) * 2]);
        if (soup.has_vertex_normal) {
          ins.read(&read_buffer[0], sizeof(float) * 3);
          vertex.normal.x = *(float*)(&read_buffer[0]);
          vertex.normal.y = *(float*)(&read_buffer[sizeof(float)]);
          vertex.normal.z = *(float*)(&read_buffer[sizeof(float) * 2]);
        }
        if (soup.has_vertex_texcoord) {
          ins.read(&read_buffer[0], sizeof(float) * 2);
          vertex.tex.x = *(float*)(&read_buffer[0]);
          vertex.tex.y = *(float*)(&read_buffer[sizeof(float)]);
        }
        if (soup.has_vertex_color) {
          ins.read(&read_buffer[0], sizeof(uint8_t) * 3);
          vertex.color.rgba[0] = *(uint8_t*)&read_buffer[0];
          vertex.color.rgba[1] = *(uint8_t*)&read_buffer[sizeof(uint8_t)];
          vertex.color.rgba[2] = *(uint8_t*)&read_buffer[sizeof(uint8_t) * 2];
          vertex.color.rgba[3] = 0;
        }
      }
    }

    read_buffer.resize(sizeof(int) * 8);
    for (auto &face : soup.faces) {
      if (is_ascii) {
        std::getline(ins, line);
        if (line.length() == 0)
          continue;
        boost::algorithm::split(str_vec, line, boost::algorithm::is_any_of(" "));
        try {
          auto num_vertex = (size_t)std::stoi(str_vec[0]);
          size_t total_size = 1 + num_vertex + (soup.has_face_texcoord ? (1 + 2 * num_vertex) : 0);
          if (str_vec.size() < total_size)
            return false;
          face.indices.resize(num_vertex);
          face.texcoords.resize(num_vertex);
        } catch (const std::exception &e) {
          MLOG("%s\n", e.what());
          return false;
        }

        try {
          for (size_t i = 0; i < face.indices.size(); ++i) {
            face.indices[i] = (size_t)std::stoi(str_vec[i + 1]);
            if (soup.has_face_texcoord) {
              face.texcoords[i].x = std::stof(str_vec[face.indices.size() + 1 + 2 * i]);
              face.texcoords[i].y = std::stof(str_vec[face.indices.size() + 1 + 2 * i + 1]);
            }
          }
        } catch (const std::exception &e) {
          MLOG("%s\n", e.what());
          return false;
        }
      } else if (is_little_endian) {
        uint8_t num_vertex = 0;
        ins.read((char*)&num_vertex, sizeof(uint8_t));

        ins.read(&read_buffer[0], sizeof(int) * num_vertex);
        face.indices.resize(num_vertex);
        for (uint8_t i = 0; i < num_vertex; ++i) {
          face.indices[i] = *(int*)(&read_buffer[i * sizeof(int)]);
        }

        if (soup.has_face_texcoord) {
          face.texcoords.resize(num_vertex);
          ins.read(&read_buffer[0], sizeof(uint8_t) + sizeof(float) * num_vertex);
          for (uint8_t i = 0; i < num_vertex; ++i) {
            face.texcoords[i].x = *(float*)(&read_buffer[sizeof(uint8_t) + i * sizeof(float) * 2]);
            face.texcoords[i].y = *(float*)(&read_buffer[sizeof(uint8_t) + i * sizeof(float) * 2 + 1]);
          }
        }
      }
    }
    return true;
  }

}
