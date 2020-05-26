#ifndef MINEOLA_SHADER_PARSER_H
#define MINEOLA_SHADER_PARSER_H

#include <vector>
#include <string>
#include <istream>

namespace mineola { namespace shader_parser {

typedef std::vector<std::pair<std::string, std::string>> effect_defines_t;

bool ParseShader(std::istream &stream, const effect_defines_t *defines, std::string &shader_str);

bool RecursiveParseShader(std::istream &stream,
  const effect_defines_t *defines, uint32_t depth, std::string &shader_str);

}} //end namespace

#endif
