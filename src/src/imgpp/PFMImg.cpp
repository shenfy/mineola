#include <mineola/ImgppLoaders.hpp>
#include <fstream>
#include <string>
#include <mineola/Imgpp.hpp>
#include "ImgppStringParser.h"

namespace {

using namespace mineola::imgpp;

bool ParseHeader(std::istream &is, int32_t &w, int32_t &h, int &channels) {
  auto token = parser::GetFixedSizeToken(is, 2);
  if (token[0] != 'P') {
    return false;
  }

  if (token[1] == 'f') {
    channels = 1;
  } else if (token[1] == 'F') {
    char ext = is.peek();
    if (ext == '4') {
      channels = 4;
      is.get();
    } else {
      channels = 3;
    }
  } else {
    return false;
  }

  // w, h
  token = parser::GetToken(is, true);
  if (token.size() == 0) {
    return false;
  }
  w = std::stoi(token);

  token = parser::GetToken(is, true);
  if (token.size() == 0) {
    return false;
  }
  h = std::stoi(token);

  token = parser::GetToken(is, true, '#');
  if (token.size() == 0) {
    return false;
  }
  float scale = std::stof(token);

  parser::SkipCommentsAndWhitespaces(is);

  return true;
}


const char* ParseHeader(const char *buffer, uint32_t length,
  int32_t &w, int32_t &h, int &channels) {

  const char *p = buffer;
  const char *const p_end = buffer + (size_t)length;

  std::string token;
  p = parser::GetFixedSizeToken(p, p_end, 2, token);
  if (p == nullptr || token[0] != 'P') {
    return nullptr;
  }

  if (token[1] == 'f') {
    channels = 1;
  } else if (token[1] == 'F') {
    char ext = *p;
    if (ext == '4') {
      channels = 4;
      p++;
    } else {
      channels = 3;
    }
  } else {
    return nullptr;
  }

  // w, h
  p = parser::GetToken(p, p_end, true, -1, token);
  if (p == nullptr) {
    return nullptr;
  }
  w = std::stoi(token);

  p = parser::GetToken(p, p_end, true, -1, token);
  if (p == nullptr) {
    return nullptr;
  }
  h = std::stoi(token);

  p = parser::GetToken(p, p_end, true, '#', token);
  if (p == nullptr) {
    return nullptr;
  }
  float scale = std::stof(token);

  p = parser::SkipCommentsAndWhitespaces(p, p_end);

  return p;
}
}  // namespace


namespace mineola { namespace imgpp {

bool LoadPFM(const char *fn, Img &img, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn)) {
    return false;
  }

  std::ifstream infile;
  infile.open(fn, std::ios::binary);
  if (!infile.good()) {
    return false;
  }

  int32_t width = 0, height = 0;
  int channels = 0;

  if (!ParseHeader(infile, width, height, channels)) {
    return false;
  }

  // allocate space
  try {
    img.SetSize(width, height, 1, channels, sizeof(float) << 3, true);
  } catch (const std::bad_alloc &) {
    return false;
  }

  int32_t line_len = sizeof(float) * width * channels;
  if (bottom_first) {
    for (uint32_t y = 0; y < height; ++y) {
      infile.read((char*)img.ROI().PtrAt(0, y, 0), line_len);
    }
  } else {
    for (uint32_t y = 0; y < height; ++y) {
      infile.read((char*)img.ROI().PtrAt(0, height - y - 1, 0), line_len);
    }
  }

  return infile.good();
}

bool LoadPFM(const char *buffer, uint32_t length, Img &img, bool bottom_first) {
  if (buffer == nullptr || length == 0) {
    return false;
  }

  int32_t width = 0, height = 0;
  int channels = 0;

  // determine channels
  const char *p = ParseHeader(buffer, length, width, height, channels);
  if (p == nullptr) {
    return false;
  }

  // allocate space
  try {
    img.SetSize(width, height, 1, channels, sizeof(float) << 3, true);
  } catch (const std::bad_alloc &) {
    return false;
  }

  int32_t line_len = sizeof(float) * width * channels;
  if (bottom_first) {
    for (uint32_t y = 0; y < height; ++y) {
      memcpy((char*)img.ROI().PtrAt(0, y, 0), p, line_len);
      p += (size_t)line_len;
    }
  } else {
    for (uint32_t y = 0; y < height; ++y) {
      memcpy((char*)img.ROI().PtrAt(0, height - y - 1, 0), p, line_len);
      p += (size_t)line_len;
    }
  }

  return p <= buffer + length;
}

bool WritePFM(const char *fn, const ImgROI &roi, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn)) {
    return false;
  }

  std::ofstream outfile;
  outfile.open(fn, std::ios::binary);
  if (!outfile.good()) {
    return false;
  }

  char *p = 0;

  if (1 == roi.Channel()) {
    outfile << "Pf\n" << roi.Width() << " " << roi.Height() << "\n-1.000000\n";
  } else if (3 == roi.Channel()) {
    outfile << "PF\n" << roi.Width() << " " << roi.Height() << "\n-1.000000\n";
  } else if (4 == roi.Channel()) {
    outfile << "PF4\n" << roi.Width() << " " << roi.Height() << "\n-1.000000\n";
  } else {
    outfile.close();
    return false;
  }

  if (bottom_first) {
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, y, 0);
      outfile.write(p, sizeof(float) * roi.Width() * roi.Channel());
    }
  } else {
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, roi.Height() - y - 1, 0);
      outfile.write(p, sizeof(float) * roi.Width() * roi.Channel());
    }
  }

  outfile.close();
  return outfile.good();
}

}}  // namespace
