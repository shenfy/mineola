  #version 300 es
  precision mediump float;
  #include "mineola_builtin_uniforms"
  in vec3 Pos;
  in vec2 TexCoord0;
  out vec2 texcoord;
  void main(void)
  {
      vec4 pos = _model_mat * vec4(Pos, 1.0);
      texcoord = TexCoord0;
      gl_Position = _proj_view_mat * pos;
  }
