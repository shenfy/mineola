  #version 300 es
  precision mediump float;
  #include "mineola_builtin_uniforms"
  uniform sampler2D diffuse_sampler;
  in vec2 texcoord;
  out vec4 fragColor;
  void main(void)
  {
      vec4 color = texture(diffuse_sampler, texcoord);
      fragColor = color;
  }
