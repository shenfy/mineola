#version 300 es
precision mediump float;
#include "mineola_builtin_uniforms"
in vec3 Pos;
in vec3 Normal;
out vec3 pos_wc;
out vec3 normal_wc;
void main(void) {
    vec4 pos = _model_mat * vec4(Pos, 1.0);
    pos_wc = pos.xyz;
    gl_Position = _proj_view_mat * pos;

    normal_wc = mat3(transpose(inverse(_model_mat))) * Normal;
};
