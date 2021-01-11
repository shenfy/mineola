#version 300 es
precision mediump float;
#include "mineola_builtin_uniforms"
uniform sampler2DShadow shadowmap;
in vec3 pos_wc;
out vec4 color;
void main() {
  vec4 shadowmap_clip = _light_proj_mat_0 * _light_view_mat_0 * vec4(pos_wc, 1.0);
  vec3 shadowmap_vp = shadowmap_clip.xyz / shadowmap_clip.w * 0.5 + 0.5;
  color = vec4(0.0, 0.0, 0.0, 1.0 - texture(shadowmap, shadowmap_vp));
}
