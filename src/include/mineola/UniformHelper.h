#ifndef MINEOLA_UNIFORMHELPER_H
#define MINEOLA_UNIFORMHELPER_H

namespace mineola { namespace gl_uniform {

void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const float *val);
void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const int32_t *val);
void SetUniform(uint32_t type, uint32_t loc, uint32_t length, const uint32_t *val);

}} //namespace

#endif
