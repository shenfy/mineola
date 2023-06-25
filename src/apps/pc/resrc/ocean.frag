#version 300 es
precision mediump float;
#include "mineola_builtin_uniforms"
in vec3 pos_wc;
in vec3 normal_wc;
out vec4 fragColor;
void main(void) {
  const float PI = 3.1415926535897932;
  const vec3 upwelling = vec3(0.0, 0.2, 0.3);
  const vec3 sky = vec3(0.69, 0.84, 1.0);
  const vec3 air = vec3(0.1, 0.1, 0.1);
  const float kDiffuse = 0.91;
  const float kNSnell = 1.34;

  vec3 n = normalize(normal_wc);
  float n_dot_l = dot(n, normalize(_light_pos_0.xyz));

  float cos_theta_i = abs(n_dot_l);
  float theta_i = acos(cos_theta_i);
  float sin_theta_t = sin(theta_i) / kNSnell;
  float theta_t = asin(sin_theta_t);

  float reflectivity;
  if (theta_i == 0.0) {
    reflectivity = (kNSnell - 1.0) / (kNSnell + 1.0);
    reflectivity *= reflectivity;
  } else {
    float fs = sin(theta_t - theta_i) / sin(theta_t + theta_i);
    float ts = tan(theta_i - theta_i) / tan(theta_i + theta_i);
    reflectivity = 0.5 * (fs * fs + ts * ts);
  }

  vec4 eye_wc = _view_mat_inv * vec4(0.0, 0.0, 0.0, 1.0);
  eye_wc /= eye_wc.w;
  vec3 d_P_E = pos_wc - eye_wc.xyz;
  float dist = 0.5 * length(d_P_E) * kDiffuse;
  dist = exp(-dist);

  fragColor = vec4(dist * (reflectivity * sky + (1.0 - reflectivity) * upwelling)
    + (1.0 - dist) * air, 1.0);
}
