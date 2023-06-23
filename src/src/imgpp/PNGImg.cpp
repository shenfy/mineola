#include <mineola/ImgppLoaders.hpp>
#include <png.h>
#include <mineola/Imgpp.hpp>

namespace mineola {
namespace imgpp {

  bool PNGAbort(const char * s) {
    printf("%s\n", s);
    return false;
  }

  bool LoadPNG(const char *fn, Img &img, bool bottom_first) {
    if (nullptr == fn || 0 == strlen(fn)) {
      return false;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes = 0;
    png_bytep *row_pointers = nullptr;

    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(fn, "rb");
    if (!fp) {
      return PNGAbort("imgpp: file could not be opened for reading!");
    }
    auto count = fread(header, 1, 8, fp);
    if (count < 8 || png_sig_cmp((unsigned char*)header, 0, 8)) {
      fclose(fp);
      return PNGAbort("imgpp: file is not recognized as a PNG file!");
    }

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr) {
      fclose(fp);
      return PNGAbort("imgpp: png_create_read_struct failed!");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, nullptr, nullptr);
      fclose(fp);
      return PNGAbort("imgpp: png_create_info_struct failed!");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      fclose(fp);
      return PNGAbort("imgpp: error setting up libpng!");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte filter_method = png_get_filter_type(png_ptr, info_ptr);
    png_byte compression_type = png_get_compression_type(png_ptr, info_ptr);
    png_byte interlace_type = png_get_interlace_type(png_ptr, info_ptr);
    if (interlace_type != 0) {  // not supporting interlacing
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      fclose(fp);
      return PNGAbort("imgpp: no support for interlaced PNG!");
    }

    /* read file */
    row_pointers = new png_bytep[height];

    int num_channel = 0;
    if (color_type == PNG_COLOR_TYPE_GRAY) {
      if (bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
      }
      num_channel = 1;
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      num_channel = 2;
    } else if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
      num_channel = 3;
    } else if (color_type == PNG_COLOR_TYPE_RGB) {
      num_channel = 3;
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
      num_channel = 4;
    }

    img.SetSize(width, height, 1, num_channel, bit_depth);
    if (bottom_first){
      for (int y = 0; y < height; y++) {
        row_pointers[y] = (unsigned char*)img.ROI().PtrAt(0, height - y - 1, 0);
      }
    } else {
      for (int y = 0; y < height; y++) {
        row_pointers[y] = (unsigned char*)img.ROI().PtrAt(0, y, 0);
      }
    }

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    png_read_image(png_ptr, row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    fclose(fp);
    delete []row_pointers;
    return true;
  }

  bool WritePNG(const char *fn, const ImgROI &roi, bool bottom_first) {
    if (nullptr == fn || 0 == strlen(fn))
      return false;

    /* create file */
    FILE *fp = fopen(fn, "wb");
    if (!fp) {
      return PNGAbort("imgpp: file could not be opened for writing!");
    }

    /* initialize stuff */
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr) {
      png_destroy_write_struct(&png_ptr, nullptr);
      return PNGAbort("imgpp: png_create_write_struct failed!");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: png_create_info_struct failed!");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: Error during init_io!");
    }

    png_init_io(png_ptr, fp);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error writing header!");
    }

    png_byte color_type;
    if (1 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_GRAY;
    } else if (2 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
    } else if (3 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_RGB;
    } else if (4 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error determining color type!");
    }

    png_set_IHDR(png_ptr, info_ptr, roi.Width(), roi.Height(),
      roi.BPC(), color_type, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr))) {
      return PNGAbort("imgpp: error during writing bytes!");
    }

    png_bytep * row_pointers = new png_bytep[roi.Height()];

    if (bottom_first) {
      for (uint32_t y = 0; y < roi.Height(); ++y) {
        row_pointers[y] = (unsigned char *)roi.PtrAt(0, roi.Height() - y - 1, 0);
      }
    } else {
      for (uint32_t y = 0; y < roi.Height(); ++y) {
        row_pointers[y] = (unsigned char *)roi.PtrAt(0, y, 0);
      }
    }

    png_write_image(png_ptr, row_pointers);

    /* end write */
    if (setjmp(png_jmpbuf(png_ptr))) {
      return PNGAbort("imgpp: error during end of write!");
    }

    png_write_end(png_ptr, nullptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    /* cleanup heap allocation */
    delete []row_pointers;

    fclose(fp);
    return true;
  }

  struct png_io_t {
    void *buffer;
    size_t length_left;
  };

  void png_copy_from_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_io_t *png_io = (png_io_t*)png_get_io_ptr(png_ptr);
    if (!png_io->buffer || png_io->length_left < length) {
      png_error(png_ptr, "EOF");
      return;
    }

    memcpy(data, png_io->buffer, length);
    png_io->buffer = (png_bytep)png_io->buffer + length;
    png_io->length_left -= length;
  }

  void png_copy_to_buffer(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_io_t *png_io = (png_io_t*)png_get_io_ptr(png_ptr);
    if (!png_io->buffer || png_io->length_left < length) {
      png_io->buffer = nullptr;
      return;
    }

    memcpy(png_io->buffer, data, length);
    png_io->buffer = (png_bytep)png_io->buffer + length;
    png_io->length_left -= length;
  }

  void png_flush_buffer(png_structp) {}

  size_t CompressPNG(const ImgROI& roi, void *dst, size_t length) {
    /* initialize stuff */
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr) {
      png_destroy_write_struct(&png_ptr, nullptr);
      return PNGAbort("imgpp: png_create_write_struct failed!");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: png_create_info_struct failed!");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error during set_write_fn!");
    }

    png_io_t png_io;
    png_io.buffer = dst;
    png_io.length_left = length;
    png_set_write_fn(png_ptr, &png_io, png_copy_to_buffer, png_flush_buffer);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error during writing header!");
    }

    png_byte color_type;
    if (1 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_GRAY;
    } else if (2 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
    } else if (3 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_RGB;
    } else if (4 == roi.Channel()) {
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    } else {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error determining color type!");
    }

    png_set_IHDR(png_ptr, info_ptr, roi.Width(), roi.Height(),
      roi.BPC(), color_type, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error during writing bytes!");
    }

    png_bytep *row_pointers = new png_bytep[roi.Height()];
    if (row_pointers == nullptr) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: failed to allocate memory!");
    }

    for (uint32_t y = 0; y < roi.Height(); ++y) {
      row_pointers[y] = (unsigned char *)roi.PtrAt(0, y, 0);
    }

    png_write_image(png_ptr, row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return PNGAbort("imgpp: error during end of write!");
    }

    png_write_end(png_ptr, nullptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    /* cleanup heap allocation */
    delete[]row_pointers;

    return (png_io.buffer != nullptr) ? (length - png_io.length_left) : 0;
  }

  bool LoadPNG(void *src, uint32_t length, Img &img, bool bottom_first) {
    if (src == nullptr || length == 0) {
      return false;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes = 0;
    png_bytep *row_pointers = nullptr;

    uint8_t header[8];
    memcpy(header, src, 8);

    if (png_sig_cmp(header, 0, 8)) {
      return PNGAbort("imgpp: file is not recognized as a PNG file!");
    }

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr) {
      png_destroy_read_struct(&png_ptr, nullptr, nullptr);
      return PNGAbort("imgpp: png_create_read_struct failed!");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      return PNGAbort("imgpp: png_create_info_struct failed!");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      return PNGAbort("imgpp: error during init_io!");
    }

    png_io_t png_io;
    png_io.buffer = src;
    png_io.length_left = length;
    png_set_read_fn(png_ptr, &png_io, png_copy_from_buffer);
    png_set_sig_bytes(png_ptr, 0);

    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte filter_method = png_get_filter_type(png_ptr, info_ptr);
    png_byte compression_type = png_get_compression_type(png_ptr, info_ptr);
    png_byte interlace_type = png_get_interlace_type(png_ptr, info_ptr);
    if (interlace_type != 0) {
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      return PNGAbort("imgpp: no support for interlaced PNG!");
    }

    row_pointers = new png_bytep[height];

    int num_channel = 0;
    if (color_type == PNG_COLOR_TYPE_GRAY) {
      if (bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
      }
      num_channel = 1;
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      num_channel = 2;
    } else if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
      num_channel = 3;
    } else if (color_type == PNG_COLOR_TYPE_RGB) {
      num_channel = 3;
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
      num_channel = 4;
    }

    img.SetSize(width, height, 1, num_channel, bit_depth);
    if (bottom_first) {
      for (int y = 0; y < height; y++) {
        row_pointers[y] = (unsigned char*)img.ROI().PtrAt(0, height - y - 1, 0);
      }
    } else {
      for (int y = 0; y < height; y++) {
        row_pointers[y] = (unsigned char*)img.ROI().PtrAt(0, y, 0);
      }
    }

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    png_read_image(png_ptr, row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    delete []row_pointers;
    return true;
  }

}}  // namespaces