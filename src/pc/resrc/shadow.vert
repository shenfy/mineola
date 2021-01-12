#version 300 es
precision mediump float;
precision highp sampler2DShadow;
#include "mineola_builtin_uniforms"
in vec3 Pos;
out vec3 pos_wc;
void main() {
  vec4 pos = _model_mat * vec4(Pos, 1.0);
  pos_wc = pos.xyz / pos.w;
  gl_Position = _proj_view_mat * pos;
}
