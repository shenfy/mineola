#version 300 es
precision mediump float;
#include "mineola_builtin_uniforms"
uniform float specularity;
uniform vec3 diffuse;
in vec3 pos_wc;
in vec3 normal_wc;
out vec4 fragColor;
void main(void) {
    // diffuse l0
    vec3 L_wc = normalize(_light_pos_0.xyz - pos_wc);
    float NdotL = max(dot(L_wc, normal_wc), 0.0);
    vec3 intensity = NdotL * _light_intensity_0.xyz;
    //specular l0
    vec3 eye_wc = _view_mat_inv[3].xyz;
    vec3 V_wc = normalize(eye_wc - pos_wc);
    vec3 H_wc = normalize(L_wc + V_wc);
    float NdotH = max(dot(H_wc, normal_wc), 0.0);
    intensity += pow(NdotH, specularity) * _light_intensity_0.xyz;

    // diffuse l1
    L_wc = normalize(_light_pos_1.xyz - pos_wc);
    NdotL = max(dot(L_wc, normal_wc), 0.0);
    intensity += NdotL * _light_intensity_1.xyz;
    // specular l1
    H_wc = normalize(L_wc + V_wc);
    NdotH = max(dot(H_wc, normal_wc), 0.0);
    intensity += pow(NdotH, specularity) * _light_intensity_1.xyz;

    intensity += 0.2;

    fragColor = vec4(clamp(intensity, 0.0, 1.0) * diffuse, 1.0);
}
