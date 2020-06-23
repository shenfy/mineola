#version 300 es
precision mediump float;
#include "mineola_builtin_uniforms"
layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
out vec3 pos_wc;
out vec3 normal_wc;

vec3 Dir2WC(vec3 dir) {
  return normalize((_model_mat * vec4(normalize(dir), 0.0)).xyz);
}

void main(void) {
  vec4 pos = _model_mat * vec4(Pos, 1.0);
  pos_wc = pos.xyz / pos.w;
  normal_wc = Dir2WC(Normal);
  gl_Position = _proj_view_mat * pos;
}
