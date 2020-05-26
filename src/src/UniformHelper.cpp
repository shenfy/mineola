#include "prefix.h"
#include <mineola/glutility.h>
#include <mineola/UniformHelper.h>

namespace mineola { namespace gl_uniform {

void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const float *val) {
  if (GL_FLOAT == type)
    glUniform1fv(loc, length, val);
  else if (GL_FLOAT_VEC2 == type)
    glUniform2fv(loc, length, val);
  else if (GL_FLOAT_VEC3 == type)
    glUniform3fv(loc, length, val);
  else if (GL_FLOAT_VEC4 == type)
    glUniform4fv(loc, length, val);
  else if (GL_FLOAT_MAT2 == type)
    glUniformMatrix2fv(loc, length, false, val);
  else if (GL_FLOAT_MAT3 == type)
    glUniformMatrix3fv(loc, length, false, val);
  else if (GL_FLOAT_MAT4 == type)
    glUniformMatrix4fv(loc, length, false, val);
  else if (GL_FLOAT_MAT2x3 == type)
    glUniformMatrix2x3fv(loc, length, false, val);
  else if (GL_FLOAT_MAT2x4 == type)
    glUniformMatrix2x4fv(loc, length, false, val);
  else if (GL_FLOAT_MAT3x2 == type)
    glUniformMatrix3x2fv(loc, length, false, val);
  else if (GL_FLOAT_MAT3x4 == type)
    glUniformMatrix3x4fv(loc, length, false, val);
  else if (GL_FLOAT_MAT4x2 == type)
    glUniformMatrix4x2fv(loc, length, false, val);
  else if (GL_FLOAT_MAT4x3 == type)
    glUniformMatrix4x3fv(loc, length, false, val);
  else
    MLOG("Shader variable type doesn't match!\n");

}

void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const int32_t *val) {
  switch (type) {
    case GL_INT:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      glUniform1iv(loc, length, val);
      break;
    case GL_INT_VEC2:
      glUniform2iv(loc, length, val);
      break;
    case GL_INT_VEC3:
      glUniform3iv(loc, length, val);
      break;
    case GL_INT_VEC4:
      glUniform4iv(loc, length, val);
      break;
    default:
      MLOG("Shader variable type doesn't match!\n");
      break;
  }
}

void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const uint32_t *val) {
  if (GL_UNSIGNED_INT == type)
    glUniform1uiv(loc, length, val);
  else if (GL_UNSIGNED_INT_VEC2 == type)
    glUniform2uiv(loc, length, val);
  else if (GL_UNSIGNED_INT_VEC3 == type)
    glUniform3uiv(loc, length, val);
  else if (GL_UNSIGNED_INT_VEC4 == type)
    glUniform4uiv(loc, length, val);
  else
    MLOG("Shader variable type doesn't match!\n");
}

}}
