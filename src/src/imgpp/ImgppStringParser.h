#ifndef IMGPP_STRINGPARSER_H
#define IMGPP_STRINGPARSER_H

#include <cctype>

namespace mineola {
namespace imgpp { namespace parser {

inline std::string GetFixedSizeToken(std::istream &is, size_t length) {
  std::string token;
  token.resize(length);
  is.get((char*)token.data(), length + 1);
  return token;
}

inline void SkipWhitespaces(std::istream &is) {
  while (true) {
    auto next_char = is.peek();
    if (next_char == std::istream::traits_type::eof()
      || !std::isspace(next_char)) {
      break;
    }
    is.get();
  }
}

inline void SkipCommentsAndWhitespaces(std::istream &is) {
  do {
    auto next_char = is.peek();
    if (next_char == '#') {  // is comment
      std::string tmp_str;
      std::getline(is, tmp_str);
    } else if (std::isspace(next_char)) {
      is.get();
    } else {
      break;
    }
  } while (true);
}

inline std::string GetToken(std::istream &is, bool skip_comments_and_whitespaces, char extra_delim = -1) {
  std::string token;
  char next_char = 0;

  if (skip_comments_and_whitespaces) {
    SkipCommentsAndWhitespaces(is);
  }

  if (!is.good()) {
    return token;
  }

  // get data
  while (true) {
    next_char = is.peek();  // peek into the future
    if (next_char == std::istream::traits_type::eof()
      || std::isspace(next_char) || next_char == extra_delim) {
      break;
    }
    token.push_back(next_char);
    is.get();  // consume the char
  }

  return token;
}

//////////////////////////////////

inline const char *GetFixedSizeToken(
  const char *p, const char *p_end, size_t length, std::string &token) {
  if (p + length >= p_end) {
    return nullptr;
  }

  token = std::move(std::string{p, length});
  return p + length;
}

inline const char *SkipWhitespaces(const char *p, const char *p_end) {
  const char *p_out = p;
  while (p_out < p_end && std::isspace(*p_out)) {
    p_out++;
  }
  return p_out;
}

inline const char *SkipCommentsAndWhitespaces(const char *p, const char *p_end) {
  const char *p_out = p;

  do {
    auto cur_char = *p_out;
    if (cur_char == '#') {  // is comment
      while(p_out < p_end && *p_out != '\n') {
        p_out++;
      }
      if (p_out < p_end) {
        p_out++;
      }
    } else if (std::isspace(cur_char)) {
      p_out++;
    } else {
      break;
    }
  } while (true);

  if (p_out < p_end) {
    return p_out;
  } else {
    return nullptr;
  }
}

inline const char *GetToken(const char *p, const char *p_end,
  bool skip_comments_and_whitespaces, char extra_delim, std::string &token) {

  const char *p_out = p;
  if (skip_comments_and_whitespaces) {
    p_out = SkipCommentsAndWhitespaces(p_out, p_end);
  }

  if (p_out >= p_end) {
    return nullptr;
  }
  token.clear();

  // get data
  while (p_out < p_end) {
    char cur_char = *p_out;  // peek into the future
    if (std::isspace(cur_char) || cur_char == extra_delim) {
      break;
    }
    token.push_back(cur_char);
    p_out++;
  }

  if (p_out < p_end) {
    return p_out;
  } else {
    return nullptr;
  }
}

// helper function for parsing the pXm file ascii headers
inline void GetNextElement(std::istream &iss, std::string &element) {
  element.clear();
  char next_char = 0;
  std::string tmp_str;
  while (true) {
    // skip white spaces
    do {
      iss.get(next_char);
    } while (std::isspace(next_char));

    if (next_char == '#') {  // skip comment until next line
      std::getline(iss, tmp_str);
      continue;
    }

    element.push_back(next_char);
    do {
      iss.get(next_char);
      element.push_back(next_char);
    } while (std::isalnum(next_char) || next_char == '.');
    break;
  }
}

// helper function for parsing the pXm file ascii headers
inline const char *GetNextElement(const char *p, const char *p_end, std::string &element) {
  element.clear();
  char cur_char = 0;
  const char *p_out = p;
  std::string tmp_str;
  while (p_out <= p_end) {
    // skip white spaces
    do {
      cur_char = *p_out;
      p_out++;
    } while (p_out <= p_end && std::isspace(cur_char));

    if (cur_char == '#') {  // skip comment until next line
      while (*p_out != '\n') {
        p_out++;
      }
      p_out++;
      continue;
    }

    element.push_back(cur_char);
    do {
      cur_char = *p_out;
      p_out++;
      element.push_back(cur_char);
    } while (p_out <= p_end && (std::isalnum(cur_char) || cur_char == '.'));
    break;
  }

  if (p_out <= p_end) {
    return p_out;
  } else {
    return nullptr;
  }
}

}}}  // namespaces

#endif
