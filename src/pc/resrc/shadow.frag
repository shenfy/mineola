#version 300 es
precision highp float;
precision highp sampler2DShadow;
#include "mineola_builtin_uniforms"
#include "mineola_pcf_soft_shadow"
in vec3 pos_wc;
out vec4 color;
void main() {
  color = vec4(0.0, 0.0, 0.0, 1.0 - PCFSoftShadow(pos_wc, 0.0));
}
