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
        // 4 lights
        vec4 _light_pos_0;
        vec4 _light_intensity_0;
        vec4 _light_pos_1;
        vec4 _light_intensity_1;
        vec4 _light_pos_2;
        vec4 _light_intensity_2;
        vec4 _light_pos_3;
        vec4 _light_intensity_3;
        // a rotational matrix
        mat4 _env_light_mat_0;
        vec4 _env_light_sh3_0[9];
        // in milliseconds
        vec4 _time;
        vec4 _delta_time;
      };
      uniform mat4 _model_mat;
      uniform sampler2DShadow _shadowmap0;
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

  void InsertHardShadowSnippet(std::string &shader_str) {
    static const char hard_shadow[] = R"(
    // 0.0 means totally in shadow, 1.0 means totally out of shadow
    float HardShadow(vec3 pos_wc, float bias) {
      vec4 shadowmap_clip = _light_proj_mat_0 * _light_view_mat_0 * vec4(pos_wc, 1.0);
      vec3 shadowmap_vp = shadowmap_clip.xyz / shadowmap_clip.w * 0.5 + 0.5;
      shadowmap_vp.z += bias;
      return texture(_shadowmap0, shadowmap_vp);
    }
    )";
    shader_str += hard_shadow;
  }

  void InsertPCFSoftShadowSnippet(std::string &shader_str) {
    static const char soft_shadow[] = R"(
    #ifndef PCF_SOFT_NUM_SAMPLES
    #define PCF_SOFT_NUM_SAMPLES 20
    #endif

    #ifndef PCF_SOFT_NUM_RINGS
    #define PCF_SOFT_NUM_RINGS 7
    #endif

    #ifndef PCF_SOFT_DISK_RADIUS
    #define PCF_SOFT_DISK_RADIUS 30.0
    #endif

    const float PCF_SOFT_PI = 3.14159265359;
    const float PCF_SOFT_PI2 = 6.28318530718;

    highp float PCFSoftRand( const in vec2 uv ) {
      const highp float a = 12.9898, b = 78.233, c = 43758.5453;
      highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PCF_SOFT_PI );
      return fract(sin(sn) * c);
    }

    vec2 poisson_disk_[PCF_SOFT_NUM_SAMPLES];

    void InitPoissonSamples( const in vec2 random_seed ) {
      float angle_step = PCF_SOFT_PI2 * float( PCF_SOFT_NUM_RINGS ) / float( PCF_SOFT_NUM_SAMPLES );
      float radius_step = 1.0 / float( PCF_SOFT_NUM_SAMPLES );
      float angle = PCFSoftRand( random_seed ) * PCF_SOFT_PI2;
      float radius = radius_step;
      // jsfiddle that shows sample pattern: https://jsfiddle.net/a16ff1p7/
      for( int i = 0; i < PCF_SOFT_NUM_SAMPLES; i ++ ) {
        poisson_disk_[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radius_step;
        angle += angle_step;
      }
    }

    // 0.0 means totally in shadow, 1.0 means totally out of shadow
    float PCFSoftShadow(vec3 pos_wc, float bias) {
      vec4 shadowmap_clip = _light_proj_mat_0 * _light_view_mat_0 * vec4(pos_wc, 1.0);
      vec3 shadowmap_vp = shadowmap_clip.xyz / shadowmap_clip.w * 0.5 + 0.5;
      shadowmap_vp.z += bias;

      InitPoissonSamples(shadowmap_vp.xy);

      float shadow = 0.0;
      vec2 texel_size = 1.0 / vec2(textureSize(_shadowmap0, 0));
      for( int i = 0; i < PCF_SOFT_NUM_SAMPLES; i ++ ) {
        vec2 coord = poisson_disk_[ i ] * texel_size.x * PCF_SOFT_DISK_RADIUS;
        vec2 coord_flip = -poisson_disk_[ i ].yx * texel_size.x * PCF_SOFT_DISK_RADIUS;
        shadow += texture(_shadowmap0, vec3(shadowmap_vp.xy + coord, shadowmap_vp.z));
        shadow += texture(_shadowmap0, vec3(shadowmap_vp.xy + coord_flip, shadowmap_vp.z));
      }
      shadow = shadow / ( 2.0 * float( PCF_SOFT_NUM_SAMPLES ) );
      return shadow;
    }
    )";
    shader_str += soft_shadow;
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
        if (defines) {
          for (auto &pair : *defines) {
            if (include_filename == pair.first) {
              include_filename = pair.second;
            }
          }
        }

        if (include_filename == "mineola_builtin_uniforms") {
          InsertBuiltInUniformBlock(shader_str);
        } else if (include_filename == "mineola_skinned_animation") {
          InsertSkinningVariables(shader_str);
        } else if (include_filename == "mineola_hard_shadow") {
          InsertHardShadowSnippet(shader_str);
        } else if (include_filename == "mineola_pcf_soft_shadow") {
          InsertPCFSoftShadowSnippet(shader_str);
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
