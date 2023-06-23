#include <mineola/ImgppLoaders.hpp>
#include <fstream>
#include <mineola/Imgpp.hpp>

namespace {

struct IMPPHeader {
  uint32_t width{0};
  uint32_t height{0};
  uint32_t channel{0};
  uint32_t depth{0};
  uint32_t bpc{0};
  uint32_t pitch{0};
  uint32_t slice_pitch{0};
  bool is_signed{false}, is_float{false};
  uint32_t buffer_length{0};
};

std::pair<std::string, int32_t> ParseElement(const char *&p) {
  uint8_t type = *(uint8_t*)p;
  if (type != 0x10 && type != 0x8) {
    return {{}, -1};
  }
  p += 1;

  std::string e_name;
  while (*p != 0) {
    e_name.push_back(*p);
    p += 1;
  }
  p += 1;  // skip 0

  int32_t result = 0;

  if (type == 0x8) {
    result = *p;
    p += 1;
  } else {
    result = *(int32_t*)p;
    p += sizeof(int32_t);
  }
  return {std::move(e_name), result};
}

int32_t ParseDataElement(const char *&p) {
  uint8_t type = *(uint8_t*)p;
  if (type != 0x5) {
    return 0;
  }

  std::string e_name;
  while (*p != 0) {
    e_name.push_back(*p);
    p += 1;
  }
  p += 1;  // skip 0

  int32_t buffer_len = *(int32_t*)p;
  p += sizeof(int32_t);

  auto subtype = *p;
  p += 1;

  return buffer_len;
}

void WriteElement(const std::string &name, int32_t val, std::string &doc) {
  doc.push_back('\x10');
  doc.append(name);
  doc.push_back('\x00');
  doc.append((char *)&val, sizeof(int32_t));
}

void WriteElement(const std::string &name, bool val, std::string &doc) {
  doc.push_back('\x08');
  doc.append(name);
  doc.push_back('\x00');
  doc.push_back(val ? '\x1' : '\x0');
}

void WriteElement(const std::string &name,
  const uint8_t *data, uint32_t size, std::string &doc) {
  doc.push_back('\x05');
  doc.append(name);
  doc.push_back('\x00');
  doc.append((const char*)&size, sizeof(int32_t));
  doc.push_back('\x00');
  if (data != nullptr) {
    doc.append((const char*)data, size);
  }
}

}

namespace mineola { namespace imgpp {

enum {kNumElements = 9};  // IMPP

bool LoadBSON(const char *src, uint32_t length, Img &img) {
  auto doc_len = *(const int32_t*)src;
  if (doc_len > (int32_t)length) {
    return false;
  }

  // parse header
  IMPPHeader header;
  const char *p = src + sizeof(int32_t);

  for (int i = 0; i < kNumElements; ++i) {
    auto [name, val] = ParseElement(p);
    if (name == "w") {
      header.width = (uint32_t)val;
    } else if (name == "h") {
      header.height = (uint32_t)val;
    } else if (name == "ch") {
      header.channel = (uint32_t)val;
    } else if (name == "d") {
      header.depth = (uint32_t)val;
    } else if (name == "bpc") {
      header.bpc = (uint32_t)val;
    } else if (name == "pitch") {
      header.pitch = (uint32_t)val;
    } else if (name == "slice") {
      header.slice_pitch = (uint32_t)val;
    } else if (name == "sgn") {
      header.is_signed = (bool)val;
    } else if (name == "flt") {
      header.is_float = (bool)val;
    } else {
      return false;
    }
  }

  // read data
  header.buffer_length = ParseDataElement(p);
  if (header.buffer_length <= 0) {
    return false;
  }

  ImgBuffer img_buf(header.buffer_length);
  img_buf.WriteData((const uint8_t*)p, header.buffer_length);

  ImgROI img_roi(img_buf.GetBuffer(), header.width, header.height, header.depth, header.channel,
    header.bpc, header.pitch, header.slice_pitch, header.is_float, header.is_signed);

  img.ROI() = img_roi;
  img.Data() = img_buf;

  return true;
}

bool LoadBSON(const char *fn, Img &img) {
  if (fn == nullptr) {
    return false;
  }
  std::ifstream infile(fn, std::ios::binary);
  if (!infile.good()) {
    return false;
  }

  infile.seekg(0, std::ios::end);
  auto file_size = infile.tellg();
  infile.seekg(0, std::ios::beg);
  std::string buffer;
  buffer.resize(file_size);
  infile.read((char*)buffer.data(), file_size);
  infile.close();

  return LoadBSON(buffer.data(), file_size, img);
}


bool WriteBSON(const Img &img, std::string &bson) {

  const auto &roi = img.ROI();
  bson.clear();
  bson.append("\x00\x00\x00\x00", sizeof(int32_t));
  WriteElement("w", (int32_t)roi.Width(), bson);
  WriteElement("h", (int32_t)roi.Height(), bson);
  WriteElement("ch", (int32_t)roi.Channel(), bson);
  WriteElement("d", (int32_t)roi.Depth(), bson);
  WriteElement("bpc", (int32_t)roi.BPC(), bson);
  WriteElement("pitch", (int32_t)roi.Pitch(), bson);
  WriteElement("slice", (int32_t)roi.SlicePitch(), bson);
  WriteElement("sgn", (int32_t)roi.IsSigned(), bson);
  WriteElement("flt", (int32_t)roi.IsFloat(), bson);
  WriteElement("b", img.Data().GetBuffer(), img.Data().GetLength(), bson);
  bson.push_back('\x00');

  int32_t doc_length = (uint32_t)bson.size();
  memcpy((char*)bson.data(), (char*)&doc_length, sizeof(int32_t));

  return true;
}

bool WriteBSON(const ImgROI &roi, std::string &bson) {

  // calc pitch
  auto pitch = ImgROI::CalcPitch(roi.Width(), roi.Channel(), roi.BPC(), 1);
  auto slice_pitch = pitch * roi.Height();

  bson.clear();
  bson.append("\x00\x00\x00\x00", sizeof(int32_t));
  WriteElement("w", (int32_t)roi.Width(), bson);
  WriteElement("h", (int32_t)roi.Height(), bson);
  WriteElement("ch", (int32_t)roi.Channel(), bson);
  WriteElement("d", (int32_t)roi.Depth(), bson);
  WriteElement("bpc", (int32_t)roi.BPC(), bson);
  WriteElement("pitch", (int32_t)pitch, bson);
  WriteElement("slice", (int32_t)slice_pitch, bson);
  WriteElement("sgn", (int32_t)roi.IsSigned(), bson);
  WriteElement("flt", (int32_t)roi.IsFloat(), bson);
  WriteElement("b", nullptr, slice_pitch * roi.Depth(), bson);

  for (uint32_t z = 0; z < roi.Depth(); ++z) {
    for (uint32_t y = 0; y < roi.Height(); ++y) {
      bson.append((const char*)roi.PtrAt(0, y, z, 0), pitch);
    }
  }

  bson.push_back('\x00');

  int32_t doc_length = (uint32_t)bson.size();
  memcpy((char*)bson.data(), (char*)&doc_length, sizeof(int32_t));

  return true;
}

bool WriteBSON(const char *fn, const Img &img) {
  if (fn == nullptr) {
    return false;
  }

  std::ofstream outfile(fn, std::ios::binary);
  if (!outfile.good()) {
    return false;
  }

  std::string bson;
  WriteBSON(img, bson);

  outfile.write((char*)bson.data(), bson.size());
  outfile.close();
  return true;
}

bool WriteBSON(const char *fn, const ImgROI &roi) {
  if (fn == nullptr) {
    return false;
  }

  std::ofstream outfile(fn, std::ios::binary);
  if (!outfile.good()) {
    return false;
  }

  std::string bson;
  WriteBSON(roi, bson);

  outfile.write((char*)bson.data(), bson.size());
  outfile.close();
  return true;
}


}}  // namespaces
