#include <mineola/ImgppLoaders.hpp>
#include <fstream>
#include <memory>
#include <mineola/Imgpp.hpp>

namespace {

#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
// definitions mimicing the windows header
struct __attribute__((packed)) BMPInfoHeader {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  int16_t  biPlanes;
  int16_t  biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};

struct __attribute__((packed)) BMPFileHeader {
  int16_t  bfType;
  uint32_t bfSize;
  int16_t  bfReserved1;
  int16_t  bfReserved2;
  uint32_t bfOffBits;
};

#elif defined(_MSC_VER)

#pragma pack(push, 1)

struct BMPInfoHeader {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  int16_t  biPlanes;
  int16_t  biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};

struct BMPFileHeader {
  int16_t  bfType;
  uint32_t bfSize;
  int16_t  bfReserved1;
  int16_t  bfReserved2;
  uint32_t bfOffBits;
};

#pragma pack(pop)

#endif

}

namespace mineola {
namespace imgpp {

bool LoadBMP(const char *fn, Img &img, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn)) {
    return false;
  }

  std::ifstream infile;
  infile.open(fn, std::ios::binary);
  if (!infile.good()) {
    return false;
  }

  BMPFileHeader bfh = {0};
  BMPInfoHeader bih = {0};

  infile.read((char *)&bfh, sizeof(BMPFileHeader));
  infile.read((char *)&bih, sizeof(BMPInfoHeader));

  bool input_bottom_first = bih.biHeight > 0;

  uint32_t width, bpp;
  int32_t height, line_len;
  width = bih.biWidth;
  height = abs(bih.biHeight);
  bpp = bih.biBitCount;
  line_len = (width * (bpp >> 3) + 3) >> 2 << 2;

  if (bfh.bfType != 0x4d42) {  // magic word for BMP
    return false;
  }

  img.SetSize(width, height, 1, bpp >> 3, 8, false, false);

  if (bpp == 8) {  // read and skip color table
    auto color_table = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) * 256]);
    infile.read((char *)color_table.get(), sizeof(uint32_t) * 256);
  }

  // jump to pixel data
  infile.seekg(bfh.bfOffBits, std::ios::beg);

  if (input_bottom_first == bottom_first) {
    for (uint32_t y = 0; y < (uint32_t)height; y++) {
      infile.read((char *)img.ROI().PtrAt(0, y, 0), line_len);
    }
  } else {
    for (uint32_t y = 0; y < (uint32_t)height; y++) {
      infile.read((char *)img.ROI().PtrAt(0, height - y - 1, 0), line_len);
    }
  }

  infile.close();
  return true;
}

bool LoadBMP(const char *buffer, uint32_t length, Img &img, bool bottom_first) {
  if (nullptr == buffer || 0 == length) {
    return false;
  }

  BMPFileHeader bfh = {0};
  BMPInfoHeader bih = {0};

  const char *p = buffer;

  memcpy(&bfh, p, sizeof(BMPFileHeader));
  p += sizeof(BMPFileHeader);
  memcpy(&bih, p, sizeof(BMPInfoHeader));
  p += sizeof(BMPInfoHeader);

  bool input_bottom_first = bih.biHeight > 0;

  uint32_t width, bpp;
  int32_t height, line_len;
  width = bih.biWidth;
  height = abs(bih.biHeight);
  bpp = bih.biBitCount;
  line_len = (width * (bpp >> 3) + 3) >> 2 << 2;

  img.SetSize(width, height, 1, bpp >> 3, 8, false, false);

  if (bpp == 8) {  // read and skip color table
    auto color_table = std::unique_ptr<uint8_t[]>(new uint8_t[sizeof(uint32_t) * 256]);
    memcpy((char *)color_table.get(), p, sizeof(uint32_t) * 256);
  }

  p = buffer + bfh.bfOffBits;
  if (input_bottom_first == bottom_first) {
    for (uint32_t y = 0; y < (uint32_t)height; y++) {
      memcpy((char *)img.ROI().PtrAt(0, y, 0), p, line_len);
      p += line_len;
    }
  } else {
    for (uint32_t y = 0; y < (uint32_t)height; y++) {
      memcpy((char *)img.ROI().PtrAt(0, height - y - 1, 0), p, line_len);
      p += line_len;
    }
  }
  return true;
}

bool WriteBMP(const char *fn, const ImgROI &roi, bool bottom_first) {
  if (nullptr == fn || 0 == strlen(fn))
    return false;

  std::ofstream outfile;
  outfile.open(fn, std::ios::binary);
  if (!outfile.good())
    return false;

  BMPFileHeader bfh;
  BMPInfoHeader bih;
  memset(&bfh, 0, sizeof(BMPFileHeader));
  memset(&bih, 0, sizeof(BMPInfoHeader));

  bih.biCompression = 0; //value of BI_RGB;
  bih.biHeight = (int)roi.Height();
  bih.biWidth = (int)roi.Width();
  bih.biPlanes = 1;
  bih.biSize = sizeof(BMPInfoHeader);
  bih.biBitCount = roi.BPC() * roi.Channel();
  bfh.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
  bfh.bfType = 0x4d42;
  int line_len = (roi.Width() * ((roi.BPC() * roi.Channel()) >> 3) + 3) >> 2 << 2;

  if (roi.Channel() == 1) {  //greyscale
    bfh.bfOffBits += sizeof(uint32_t) * 256;
    outfile.write((char*)&bfh, sizeof(BMPFileHeader));
    outfile.write((char*)&bih, sizeof(BMPInfoHeader));
    uint32_t color_table[256];
    uint32_t color = 0;
    for (int i = 0; i < 256; ++i, color += 0x10101)
      color_table[i] = color;
    outfile.write((char*)color_table, sizeof(uint32_t) * 256);
  } else {
    outfile.write((char*)&bfh, sizeof(BMPFileHeader));
    outfile.write((char*)&bih, sizeof(BMPInfoHeader));
  }

  char *p = 0;
  if (bottom_first) {
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, y, 0);
      outfile.write(p, line_len);
    }
  } else {
    for (uint32_t y = 0; y < roi.Height(); y++) {
      p = (char*)roi.PtrAt(0, roi.Height() - y - 1, 0);
      outfile.write(p, line_len);
    }
  }

  outfile.close();
  return true;
}

}}  // namespaces
