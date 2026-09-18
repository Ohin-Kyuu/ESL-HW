#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Testbench.h"
#include "filter_def.h"

using namespace std;

// 8-bit grayscale BMP header = 14(file) + 40(info) + 256*4(palette) = 1078
static unsigned char bmp_gray_header[1078] = {
    // BITMAPFILEHEADER (14 bytes)
    0x42, 0x4D,             // Signature "BM"
    0, 0, 0, 0,             // File size
    0, 0,                   // Reserved1
    0, 0,                   // Reserved2
    0x36, 0x04, 0x00, 0x00, // Data offset = 1078 = 0x00000436

    // BITMAPINFOHEADER (40 bytes)
    40, 0, 0, 0,            // Header size
    0, 0, 0, 0,             // Width
    0, 0, 0, 0,             // Height
    1, 0,                   // Planes
    8, 0,                   // Bits per pixel = 8
    0, 0, 0, 0,             // Compression = BI_RGB
    0, 0, 0, 0,             // Image size
    0, 0, 0, 0,             // X pixels per meter
    0, 0, 0, 0,             // Y pixels per meter
    0x00, 0x01, 0x00, 0x00, // Colors used = 256
    0x00, 0x01, 0x00, 0x00  // Important colors = 256
                            // followed by 256-color grayscale palette
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), width(0), height(0), bits_per_pixel(0),
      input_data_offset(0), gray_bitmap(nullptr), target_bitmap(nullptr) {
  SC_THREAD(do_modules);
  sensitive << i_clk.pos();
  dont_initialize();
}

int Testbench::read_bmp(const std::string &infile_name) {
  FILE *fp = fopen(infile_name.c_str(), "rb");
  if (fp == NULL) {
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }

  unsigned char file_header[14];
  unsigned char info_header[40];

  if (fread(file_header, 1, 14, fp) != 14) {
    printf("read bmp file header error\n");
    fclose(fp);
    return -1;
  }

  if (file_header[0] != 'B' || file_header[1] != 'M') {
    printf("not a bmp file: %s\n", infile_name.c_str());
    fclose(fp);
    return -1;
  }

  if (fread(info_header, 1, 40, fp) != 40) {
    printf("read bmp info header error\n");
    fclose(fp);
    return -1;
  }

  input_data_offset = *(unsigned int *)&file_header[10];
  width = *(unsigned int *)&info_header[4];
  height = *(unsigned int *)&info_header[8];
  bits_per_pixel = *(unsigned short *)&info_header[14];

  if (!(bits_per_pixel == 8 || bits_per_pixel == 24)) {
    printf("unsupported bmp bpp = %u, only 8-bit gray or 24-bit rgb bmp "
           "supported\n",
           bits_per_pixel);
    fclose(fp);
    return -1;
  }

  // BMP rows are padded to multiples of 4 bytes
  unsigned int input_row_bytes = ((width * bits_per_pixel + 31) / 32) * 4;

  unsigned char *row_buf = (unsigned char *)malloc(input_row_bytes);
  if (row_buf == NULL) {
    printf("malloc row_buf error\n");
    fclose(fp);
    return -1;
  }

  gray_bitmap = (unsigned char *)malloc((size_t)width * height);
  if (gray_bitmap == NULL) {
    printf("malloc gray_bitmap error\n");
    free(row_buf);
    fclose(fp);
    return -1;
  }

  target_bitmap = (unsigned char *)malloc((size_t)width * height);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    free(gray_bitmap);
    gray_bitmap = nullptr;
    free(row_buf);
    fclose(fp);
    return -1;
  }

  fseek(fp, input_data_offset, SEEK_SET);

  // BMP stored bottom-up by default
  for (unsigned int y = 0; y < height; ++y) {
    if (fread(row_buf, 1, input_row_bytes, fp) != input_row_bytes) {
      printf("read bmp pixel data error\n");
      free(row_buf);
      fclose(fp);
      return -1;
    }

    unsigned int dst_y = height - 1 - y;

    if (bits_per_pixel == 8) {
      for (unsigned int x = 0; x < width; ++x) {
        gray_bitmap[dst_y * width + x] = row_buf[x];
      }
    } else { // 24-bit BGR -> gray
      for (unsigned int x = 0; x < width; ++x) {
        unsigned char B = row_buf[x * 3 + 0];
        unsigned char G = row_buf[x * 3 + 1];
        unsigned char R = row_buf[x * 3 + 2];

        // integer grayscale approximation
        unsigned char gray = (unsigned char)((77 * R + 150 * G + 29 * B) >> 8);

        gray_bitmap[dst_y * width + x] = gray;
      }
    }
  }

  free(row_buf);
  fclose(fp);

  printf("Image width=%u, height=%u, bits_per_pixel=%u\n", width, height,
         bits_per_pixel);

  if ((int)width != IMG_X || (int)height != IMG_Y) {
    printf("ERROR: image size %ux%u != compile-time IMG_X x IMG_Y = %dx%d\n",
           width, height, IMG_X, IMG_Y);
    return -1;
  }

  return 0;
}

int Testbench::write_bmp(const std::string &outfile_name) {
  FILE *fp = fopen(outfile_name.c_str(), "wb");
  if (fp == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // 8-bit grayscale BMP needs row padding
  unsigned int output_row_bytes = ((width * 8 + 31) / 32) * 4;
  unsigned int image_size = output_row_bytes * height;
  unsigned int file_size = 1078 + image_size;

  unsigned char header[1078];
  memcpy(header, bmp_gray_header, sizeof(header));

  // file size
  header[2] = (unsigned char)(file_size & 0xFF);
  header[3] = (unsigned char)((file_size >> 8) & 0xFF);
  header[4] = (unsigned char)((file_size >> 16) & 0xFF);
  header[5] = (unsigned char)((file_size >> 24) & 0xFF);

  // width
  header[18] = (unsigned char)(width & 0xFF);
  header[19] = (unsigned char)((width >> 8) & 0xFF);
  header[20] = (unsigned char)((width >> 16) & 0xFF);
  header[21] = (unsigned char)((width >> 24) & 0xFF);

  // height
  header[22] = (unsigned char)(height & 0xFF);
  header[23] = (unsigned char)((height >> 8) & 0xFF);
  header[24] = (unsigned char)((height >> 16) & 0xFF);
  header[25] = (unsigned char)((height >> 24) & 0xFF);

  // image size
  header[34] = (unsigned char)(image_size & 0xFF);
  header[35] = (unsigned char)((image_size >> 8) & 0xFF);
  header[36] = (unsigned char)((image_size >> 16) & 0xFF);
  header[37] = (unsigned char)((image_size >> 24) & 0xFF);

  // write header (54 bytes)
  fwrite(header, 1, 54, fp);

  // write grayscale palette (256 entries, BGRA)
  for (int i = 0; i < 256; ++i) {
    unsigned char palette[4];
    palette[0] = (unsigned char)i; // B
    palette[1] = (unsigned char)i; // G
    palette[2] = (unsigned char)i; // R
    palette[3] = 0;
    fwrite(palette, 1, 4, fp);
  }

  unsigned char *row_buf = (unsigned char *)calloc(output_row_bytes, 1);
  if (row_buf == NULL) {
    printf("malloc output row_buf error\n");
    fclose(fp);
    return -1;
  }

  // write bottom-up
  for (unsigned int y = 0; y < height; ++y) {
    unsigned int src_y = height - 1 - y;
    memset(row_buf, 0, output_row_bytes);

    for (unsigned int x = 0; x < width; ++x) {
      row_buf[x] = target_bitmap[src_y * width + x];
    }

    fwrite(row_buf, 1, output_row_bytes, fp);
  }

  free(row_buf);
  fclose(fp);
  return 0;
}

void Testbench::do_modules() {
  int result;
  int pad = 2;

  o_rst.write(false);
  wait();
  o_rst.write(true);

  for (int y = -pad; y < (int)height + pad; ++y) {
    for (int x = -pad; x < (int)width + pad; ++x) {
      // Write pixels to socket
      sc_uint<8> pixel = 0;
      if (x >= 0 && x < (int)width && y >= 0 && y < (int)height) {
        pixel = gray_bitmap[y * width + x];
      } else {
        pixel = 0;
      }
      o_g.write(pixel);

      // Read results: total pipeline latency is 2 stages * (1 row + 1 col)
      int cur_x = x - pad;
      int cur_y = y - pad;
      if (i_result.num_available() == 0)
        wait(i_result.data_written_event());
      result = i_result.read();

      if (cur_x >= 0 && cur_x < (int)width && cur_y >= 0 &&
          cur_y < (int)height) {
        target_bitmap[cur_y * width + cur_x] = (sc_uint<8>)result;
      }
    }
  }
  sc_stop();
}
