#include <mineola/ImgppLoaders.hpp>
#include <cstring>
#include <fstream>
#include <string>
#include <mineola/Imgpp.hpp>
#include "ImgppStringParser.h"

namespace {
using namespace mineola::imgpp;

bool ParseHeader(std::istream &is, int &width, int &height, int &channel, int &bit_depth) {
  auto token = parser::GetFixedSizeToken(is, 2);
  if (token[0] != 'P') {
    return false;
  }

  if (token[1] == '5') {
    channel = 1;
  } else if (token[1] == '6') {
    channel = 3;
  } else {
    return false;
  }

  // w, h
  token = parser::GetToken(is, true);
  if (token.size() == 0) {
    return false;
  }
  width = std::stoi(token);

  token = parser::GetToken(is, true);
  if (token.size() == 0) {
    return false;
  }
  height = std::stoi(token);

  // bit-depth
  token = parser::GetToken(is, true, '#');
  if (token.size() == 0) {
    return false;
  }
  int max_val = std::stoi(token);
  bit_depth = 0;
  while (max_val != 0) {
    max_val >>= 1;
    bit_depth++;
  }
  bit_depth = (bit_depth + 7) >> 3 << 3;

  parser::SkipCommentsAndWhitespaces(is);

  return true;
}

const char *ParseHeader(const char *buffer, uint32_t length,
  int &width, int &height, int &channel, int &bit_depth) {

  const char *p = buffer;
  const char *const p_end = buffer + (size_t)length;

  std::string token;
  p = parser::GetFixedSizeToken(p, p_end, 2, token);
  if (p == nullptr || token[0] != 'P') {
    return nullptr;
  }

  if (token[1] == '5') {
    channel = 1;
  } else if (token[1] == '6') {
    channel = 3;
  } else {
    return nullptr;
  }

  // w, h
  p = parser::GetToken(p, p_end, true, -1, token);
  if (p == nullptr) {
    return nullptr;
  }
  width = std::stoi(token);

  p = parser::GetToken(p, p_end, true, -1, token);
  if (p == nullptr) {
    return nullptr;
  }
  height = std::stoi(token);

  // bit-depth
  p = parser::GetToken(p, p_end, true, '#', token);
  if (p == nullptr) {
    return nullptr;
  }
  int max_val = std::stoi(token);
  bit_depth = 0;
  while (max_val != 0) {
    max_val >>= 1;
    bit_depth++;
  }
  bit_depth = (bit_depth + 7) >> 3 << 3;

  p = parser::SkipCommentsAndWhitespaces(p, p_end);

  return p;
}
}  // namespace

namespace mineola { namespace imgpp {

bool LoadPPM(const char *fn, Img &img, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn)) {
    return false;
  }

  std::ifstream infile(fn, std::ios::binary);
  if (!infile.good()) {
    return false;
  }

  int width = 0, height = 0;
  int channel = 0;
  int bit_depth = 0;

  if (!ParseHeader(infile, width, height, channel, bit_depth)) {
    return false;
  }

  img.SetSize(width, height, 1, channel, bit_depth, false, false);

  int32_t byte_len = bit_depth >> 3;
  int32_t line_len = width * channel * byte_len;
  // pgm and ppm are both stored from top to bottom
  if (bottom_first) {
    // flip image while reading
    for (uint32_t y = 0; y < height; ++y) {
      infile.read((char*)img.ROI().PtrAt(0, height - y - 1, 0), line_len);
    }
  } else {
    for (uint32_t y = 0; y < height; ++y) {
      infile.read((char*)img.ROI().PtrAt(0, y, 0), line_len);
    }
  }

  infile.close();
  return infile.good();
}

bool LoadPPM(const char *buffer, uint32_t length, Img &img, bool bottom_first) {
  if (buffer == 0 || 0 == length) {
    return false;
  }

  int width = 0, height = 0;
  int channel = 0;
  int bit_depth = 0;

  const char *p = ParseHeader(buffer, length, width, height, channel, bit_depth);
  if (p == nullptr) {
    return false;
  }

  img.SetSize(width, height, 1, channel, bit_depth, false, false);

  int32_t byte_len = bit_depth >> 3;
  int32_t line_len = width * channel * byte_len;
  // pgm and ppm are both stored from top to bottom
  if (bottom_first) {
    for (uint32_t y = 0; y < height; ++y) {
      // flip image while reading
      memcpy((char*)img.ROI().PtrAt(0, height - y - 1, 0), p, line_len);
      p += line_len;
    }
  } else {
    for (uint32_t y = 0; y < height; ++y) {
      memcpy((char*)img.ROI().PtrAt(0, y, 0), p, line_len);
      p += line_len;
    }
  }

  return p <= buffer + length;
}

bool WritePPM(const char *fn, const ImgROI &roi, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn))
    return false;

  std::ofstream outfile;
  outfile.open(fn, std::ios::binary);
  if (!outfile.good())
    return false;

  char *p = 0;

  if (1 == roi.Channel())
    outfile << "P5\n";  // PGM
  else if (3 == roi.Channel())
    outfile << "P6\n";
  else {
    outfile.close();
    return false;
  }

  outfile << roi.Width() << " " << roi.Height() << "\n" << ((1UL << roi.BPC()) - 1) << "\n";

  int line_len = (int)roi.Pitch();

  // pgm and ppm are both stored from top to bottom
  if (bottom_first) {
    // write flipped image
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, roi.Height() - y - 1, 0);
      outfile.write(p, line_len);
    }
  } else {
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, y, 0);
      outfile.write(p, line_len);
    }
  }
  outfile.close();
  return outfile.good();
}

}}  // namespace
