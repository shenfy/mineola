#version 300 es
precision mediump float;
precision highp sampler2DShadow;
#include "mineola_builtin_uniforms"
#include "mineola_hard_shadow"
in vec3 pos_wc;
out vec4 color;
void main() {
  color = vec4(0.0, 0.0, 0.0, 1.0 - HardShadow(pos_wc, 0.0));
}
