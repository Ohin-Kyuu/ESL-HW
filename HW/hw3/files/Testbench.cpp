#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "Testbench.h"
#include "filter_def.h"

using namespace std;

#define WHITE     255
#define BLACK     0
#define THRESHOLD 90

static unsigned char bmp_gray_header[1078] = {
    0x42, 0x4D, 0,0,0,0, 0,0, 0,0, 0x36,0x04,0x00,0x00,
    40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 8,0,
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    0x00,0x01,0x00,0x00, 0x00,0x01,0x00,0x00
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), width(0), height(0), bits_per_pixel(0),
      input_data_offset(0), gray_bitmap(nullptr), target_bitmap(nullptr) {
  SC_THREAD(feed);
  sensitive << i_clk.pos();
  dont_initialize();

  SC_THREAD(fetch);
  sensitive << i_clk.pos();
  dont_initialize();

#ifndef NATIVE_SYSTEMC
  o_g.clk_rst(i_clk, o_rst);
#endif
}

Testbench::~Testbench() {
  cout << "Total run time = " << total_run_time << endl;
  if (gray_bitmap)   free(gray_bitmap);
  if (target_bitmap) free(target_bitmap);
}

int Testbench::read_bmp(const std::string &infile_name) {
  FILE *fp = fopen(infile_name.c_str(), "rb");
  if (!fp) { printf("fopen %s error\n", infile_name.c_str()); return -1; }

  unsigned char file_header[14];
  unsigned char info_header[40];
  if (fread(file_header, 1, 14, fp) != 14) { fclose(fp); return -1; }
  if (file_header[0]!='B'||file_header[1]!='M') { fclose(fp); return -1; }
  if (fread(info_header, 1, 40, fp) != 40) { fclose(fp); return -1; }

  input_data_offset = *(unsigned int  *)&file_header[10];
  width             = *(unsigned int  *)&info_header[4];
  height            = *(unsigned int  *)&info_header[8];
  bits_per_pixel    = *(unsigned short*)&info_header[14];

  if (bits_per_pixel != 8 && bits_per_pixel != 24) {
    printf("unsupported bpp=%u\n", bits_per_pixel); fclose(fp); return -1;
  }

  unsigned int row_bytes = ((width * bits_per_pixel + 31) / 32) * 4;
  unsigned char *row_buf = (unsigned char *)malloc(row_bytes);
  gray_bitmap   = (unsigned char *)malloc((size_t)width * height);
  target_bitmap = (unsigned char *)malloc((size_t)width * height);
  if (!row_buf || !gray_bitmap || !target_bitmap) {
    free(row_buf); free(gray_bitmap); free(target_bitmap);
    fclose(fp); return -1;
  }

  fseek(fp, input_data_offset, SEEK_SET);
  for (unsigned int y = 0; y < height; ++y) {
    if (fread(row_buf, 1, row_bytes, fp) != row_bytes) {
      free(row_buf); fclose(fp); return -1;
    }
    unsigned int dst_y = height - 1 - y;
    if (bits_per_pixel == 8) {
      for (unsigned int x = 0; x < width; ++x)
        gray_bitmap[dst_y * width + x] = row_buf[x];
    } else {
      for (unsigned int x = 0; x < width; ++x) {
        unsigned char B=row_buf[x*3+0], G=row_buf[x*3+1], R=row_buf[x*3+2];
        gray_bitmap[dst_y * width + x] =
            (unsigned char)((77*R + 150*G + 29*B) >> 8);
      }
    }
  }
  free(row_buf);
  fclose(fp);
  printf("Image width=%u height=%u bpp=%u\n", width, height, bits_per_pixel);
  return 0;
}

int Testbench::write_bmp(const std::string &outfile_name) {
  FILE *fp = fopen(outfile_name.c_str(), "wb");
  if (!fp) { printf("fopen %s error\n", outfile_name.c_str()); return -1; }

  unsigned int row_bytes  = ((width * 8 + 31) / 32) * 4;
  unsigned int image_size = row_bytes * height;
  unsigned int file_size  = 1078 + image_size;

  unsigned char header[1078];
  memcpy(header, bmp_gray_header, sizeof(header));
  header[2]=(file_size)&0xFF;       header[3]=(file_size>>8)&0xFF;
  header[4]=(file_size>>16)&0xFF;   header[5]=(file_size>>24)&0xFF;
  header[18]=(width)&0xFF;          header[19]=(width>>8)&0xFF;
  header[20]=(width>>16)&0xFF;      header[21]=(width>>24)&0xFF;
  header[22]=(height)&0xFF;         header[23]=(height>>8)&0xFF;
  header[24]=(height>>16)&0xFF;     header[25]=(height>>24)&0xFF;
  header[34]=(image_size)&0xFF;     header[35]=(image_size>>8)&0xFF;
  header[36]=(image_size>>16)&0xFF; header[37]=(image_size>>24)&0xFF;

  fwrite(header, 1, 54, fp);
  for (int i = 0; i < 256; ++i) {
    unsigned char p[4] = {(unsigned char)i,(unsigned char)i,(unsigned char)i,0};
    fwrite(p, 1, 4, fp);
  }

  unsigned char *row_buf = (unsigned char *)calloc(row_bytes, 1);
  for (unsigned int y = 0; y < height; ++y) {
    unsigned int src_y = height - 1 - y;
    memset(row_buf, 0, row_bytes);
    for (unsigned int x = 0; x < width; ++x)
      row_buf[x] = target_bitmap[src_y * width + x];
    fwrite(row_buf, 1, row_bytes, fp);
  }
  free(row_buf);
  fclose(fp);
  return 0;
}

void Testbench::feed() {
  HLS_RESET_OUT(o_g);
  o_rst.write(false);
  wait(5);
  o_rst.write(true);
  wait(1);

  total_start_time = sc_time_stamp();

  GLB_START(o_g);
  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      sc_dt::sc_uint<8> pixel = gray_bitmap[y * width + x];
      GLB_PUT(o_g, pixel);
    }
    GLB_NEXT_Y(o_g);
  }
  GLB_END(o_g);
}

void Testbench::fetch() {
  HLS_RESET_IN(i_result);
  wait(5);
  wait(1);

  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {
      sc_dt::sc_uint<32> total;
      HLS_READ(i_result, total);
      int result = (int)(unsigned int)total;
      target_bitmap[y * width + x] = result;
    }
  }

  total_run_time = sc_time_stamp() - total_start_time;
  sc_stop();
}
