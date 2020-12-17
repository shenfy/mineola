#include "prefix.h"
#include <mineola/ShaderParser.h>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <mineola/Engine.h>

namespace mineola { namespace shader_parser {
  const uint32_t kMaxIncludeDepth = 4;
  bool IsIncludedFile(std::string &line,
    uint32_t line_number,
    std::string &include_filename) {
    const std::string delimiters = " \f\r\t\v";
    std::size_t first_found = line.find_first_not_of(delimiters);
    if (first_found == std::string::npos) return false;
    if (line[first_found] != '#') return false;

    std::size_t second_found = line.substr(first_found).find_first_not_of(delimiters);
    second_found += first_found + 1;
    if (second_found + 7 >= line.size() || line.substr(second_found, 7) != "include") {
      return false;
    }

    std::regex e(R"("\S\S.*")");
    std::smatch m;
    std::string s = line.substr(second_found + 7);
    std::size_t third_found = s.find_first_not_of(delimiters);
    s = s.substr(third_found);
    if (!std::regex_match(s, m, e)) {
      MLOG("Include syntax error on line #%d: %s\n", line_number, line.c_str());
      return false;
    } else {
      include_filename = m[0];
      include_filename = include_filename.substr(1);
      include_filename.pop_back();
    }
    return true;
  }

  void InsertBuiltInUniformBlock(std::string &shader_str) {
    static const char built_in_uniform_str[] = R"(
      layout(std140) uniform mineola_builtin_uniforms {
        mat4 _view_mat;
        mat4 _view_mat_inv;
        mat4 _proj_mat;
        mat4 _proj_mat_inv;
        mat4 _proj_view_mat;
        mat4 _light_view_mat_0;
        mat4 _light_proj_mat_0;
        // only _viewport_size.xy have valid value
        vec4 _viewport_size;
        vec4 _light_pos_0;
        vec4 _light_intensity_0;
        // a rotational matrix
        mat4 _env_light_mat_0;
        vec4 _env_light_sh3_0[9];
        // in milliseconds
        vec4 _time;
        vec4 _delta_time;
      };
      uniform mat4 _model_mat;
      uniform sampler2D _env_light_probe_0;
    )";
    shader_str += built_in_uniform_str;
  }

  void InsertSkinningVariables(std::string &shader_str) {
    static const char skinning_uniform_str[] = R"(
      #if defined(kMaxJoints)
      #else
      #define kMaxJoints 32
      #endif
      in vec4 BlendIdx;
      in vec4 BlendWeight;
      uniform mat4 _joint_mats[kMaxJoints];
    )";
    shader_str += skinning_uniform_str;
  }

  std::string BuildDefineString(const effect_defines_t *defines) {
    if (!defines)
      return "";
    std::ostringstream result;
    for (auto &pair : *defines) {
      if (pair.second.empty()) {
        result << "#define " << pair.first << "\n";
      } else {
        result << "#define " << pair.first << " " << pair.second << "\n";
      }
    }
    return result.str();
  }

  bool ParseShader(std::istream &stream, const effect_defines_t *defines, std::string &shader_str) {
    return RecursiveParseShader(stream, defines, 0, shader_str);
  }

  bool RecursiveParseShader(std::istream &stream,
    const effect_defines_t *defines, uint32_t depth, std::string &shader_str) {
    if (depth > kMaxIncludeDepth) {
      MLOG("Include depth exceeds MAX include depth!\n");
      return false;
    }

    std::string line;
    uint32_t line_number = 0;
    while(std::getline(stream, line)) {
      line_number++;
      std::string include_filename;
      if (IsIncludedFile(line, line_number, include_filename)) {
        if (defines)
          for (auto &pair : *defines)
            if (include_filename == pair.first)
              include_filename = pair.second;

        if (include_filename == "mineola_builtin_uniforms") {
          InsertBuiltInUniformBlock(shader_str);
        } else if (include_filename == "mineola_skinned_animation") {
          InsertSkinningVariables(shader_str);
        } else {
          std::string found_fn;
          if (!Engine::Instance().ResrcMgr().LocateFile(include_filename.c_str(), found_fn)) {
            MLOG("Include file %s does not exist!\n", include_filename.c_str());
            return false;
          }
          std::ifstream infile(found_fn);
          if (!infile.good()) {
            MLOG("Failed to open %s!\n", found_fn.c_str());
            return false;
          }
          if (!RecursiveParseShader(infile, defines, depth + 1, shader_str)) {
            infile.close();
            return false;
          }
          infile.close();
        }
      } else {
        if (shader_str.empty()) {
          // before appending the first line, check whether it's a version directive
          // and insert or append define strings
          if (line.substr(0, 8) == std::string("#version")) {
            shader_str += line + "\n";
            shader_str += BuildDefineString(defines);
          } else {
            shader_str += BuildDefineString(defines);
            shader_str += line + "\n";
          }
        } else {
          shader_str += line + "\n";
        }
      }
    }

    return true;
  }
}}
