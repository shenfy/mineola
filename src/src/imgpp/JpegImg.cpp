#include <mineola/ImgppLoaders.hpp>
#include <mineola/Imgpp.hpp>
#include <mineola/ImgppLoadersExt.hpp>
#include <jpeglib.h>
#include <jerror.h>
#include <cstdio>

using namespace std;

namespace mineola {
namespace imgpp {

bool LoadJPEG(const char *fn, Img &img, bool bottom_first) {
  if (NULL == fn || 0 == strlen(fn))
    return false;

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE * infile;
  if ((infile = fopen(fn, "rb")) == NULL) {
    fprintf(stderr, "imgpp: Can't open %s!\n", fn);
    return false;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  try {
    img.SetSize(
      (uint32_t)(cinfo.output_width), (uint32_t)(cinfo.output_height),
      1UL, (uint32_t)cinfo.output_components, 8);
  } catch (const std::bad_alloc &) {
    fprintf(stderr, "imgpp: Failed to allocate memory for jpeg image!\n");
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return false;
  }

  JSAMPLE *row[1];
  if (bottom_first) {
    for (uint32_t y = 0; y < cinfo.output_height; ++y) {
      row[0] = (JSAMPLE*)img.ROI().PtrAt(0, cinfo.output_height - y - 1, 0);
      jpeg_read_scanlines(&cinfo, row, 1);
    }
  } else {
    for (uint32_t y = 0; y < cinfo.output_height; ++y) {
      row[0] = (JSAMPLE*)img.ROI().PtrAt(0, y, 0);
      jpeg_read_scanlines(&cinfo, row, 1);
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  return true;
}

bool WriteJPEG(const char *fn, const ImgROI &roi, bool bottom_first) {
  return WriteJPEGAtQuality(fn, roi, 75, bottom_first);
}

bool WriteJPEGAtQuality(const char *fn, const ImgROI &roi, int quality, bool bottom_first) {
  if (NULL == fn || 0 == strlen(fn) || quality <= 0 || quality > 100) {
    return false;
  }

  FILE * outfile;
  if ((outfile = fopen(fn, "wb")) == NULL) {
    fprintf(stderr, "imgpp: Can't open %s for writing.\n", fn);
    return false;
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, outfile);

  J_COLOR_SPACE color_space = JCS_UNKNOWN;
  if (roi.Channel() == 1)
    color_space = JCS_GRAYSCALE;
  else if (roi.Channel() == 3)
    color_space = JCS_RGB;

  cinfo.image_width = roi.Width();
  cinfo.image_height = roi.Height();
  cinfo.input_components = roi.Channel();
  cinfo.in_color_space = color_space;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, FALSE);

  jpeg_start_compress(&cinfo, TRUE);

  JSAMPLE *row[1];
  if (bottom_first) {
    for (uint32_t y = 0; y < roi.Height(); ++y) {
      row[0] = (JSAMPLE*)roi.PtrAt(0, roi.Height() - y - 1, 0);
      jpeg_write_scanlines(&cinfo, row, 1);
    }
  } else {
    for (uint32_t y = 0; y < roi.Height(); ++y) {
      row[0] = (JSAMPLE*)roi.PtrAt(0, y, 0);
      jpeg_write_scanlines(&cinfo, row, 1);
    }
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  if (outfile != NULL) {
    fclose(outfile);
  }
  return true;
}


bool LoadJPEG(const void *src, uint32_t length, Img &img, bool bottom_first) {
  if (src == nullptr || length == 0) {
    return false;
  }

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, (uint8_t*)src, length);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  try {
    img.SetSize(
      (uint32_t)(cinfo.output_width), (uint32_t)(cinfo.output_height),
      1UL, (uint32_t)cinfo.output_components, 8);
  } catch (std::bad_alloc &) {
    fprintf(stderr, "imgpp: Failed to allocate memory for jpeg image!\n");
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  JSAMPLE *row[1];
  if (bottom_first) {
    for (uint32_t y = 0; y < cinfo.output_height; ++y) {
      row[0] = (JSAMPLE*)img.ROI().PtrAt(0, cinfo.output_height - y - 1, 0);
      jpeg_read_scanlines(&cinfo, row, 1);
    }
  } else {
    for (uint32_t y = 0; y < cinfo.output_height; ++y) {
      row[0] = (JSAMPLE*)img.ROI().PtrAt(0, y, 0);
      jpeg_read_scanlines(&cinfo, row, 1);
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return true;
}

uint32_t CompressJPEG(const ImgROI &roi, void *dst, uint32_t length) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  unsigned char *compressed_buffer = nullptr;
  unsigned long compressed_length = 0;
  jpeg_mem_dest(&cinfo, &compressed_buffer, &compressed_length);

  if (length < compressed_length) {
    free(compressed_buffer);
    return 0;
  }

  J_COLOR_SPACE color_space = JCS_UNKNOWN;
  if (roi.Channel() == 1)
      color_space = JCS_GRAYSCALE;
  else if (roi.Channel() == 3)
      color_space = JCS_RGB;

  cinfo.image_width = roi.Width();
  cinfo.image_height = roi.Height();
  cinfo.input_components = roi.Channel();
  cinfo.in_color_space = color_space;
  jpeg_set_defaults(&cinfo);

  jpeg_start_compress(&cinfo, TRUE);

  JSAMPLE *row[1];
  for (uint32_t y = 0; y < roi.Height(); ++y) {
    row[0] = (JSAMPLE*)roi.PtrAt(0, y, 0);
    jpeg_write_scanlines(&cinfo, row, 1);
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  memcpy(dst, compressed_buffer, compressed_length);
  free(compressed_buffer);
  return compressed_length;
}

}}  // end namespace
