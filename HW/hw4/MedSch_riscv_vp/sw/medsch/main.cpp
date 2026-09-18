#include "Testbench.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifndef INPUT_BMP
#define INPUT_BMP "cameraman.bmp" // default image
#endif
#define OUTPUT_BMP "out_" INPUT_BMP

unsigned int width;
unsigned int height;
unsigned short bits_per_pixel;
unsigned int input_data_offset;
unsigned char *gray_bitmap;   // internal grayscale input image
unsigned char *target_bitmap; // grayscale output image

bool _is_using_dma = false;

int read_bmp(const std::string &infile_name) {
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

  // input_data_offset = *(unsigned int *)&file_header[10];
  // width = *(unsigned int *)&info_header[4];
  // height = *(unsigned int *)&info_header[8];
  // bits_per_pixel = *(unsigned short *)&info_header[14];
  input_data_offset = file_header[10] | (file_header[11] << 8) |
                      (file_header[12] << 16) | (file_header[13] << 24);

  width = info_header[4] | (info_header[5] << 8) | (info_header[6] << 16) |
          (info_header[7] << 24);

  height = info_header[8] | (info_header[9] << 8) | (info_header[10] << 16) |
           (info_header[11] << 24);

  bits_per_pixel = info_header[14] | (info_header[15] << 8);

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

  return 0;
}

int write_bmp(const std::string &outfile_name) {
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

inline unsigned int read_cycles() {
  unsigned int cycles;
  asm volatile("csrr %0, mcycle" : "=r"(cycles));
  return cycles;
}

void dma(uint32_t src, uint32_t dst, uint32_t len) {
  *DMA_SRC_ADDR = src;
  *DMA_DST_ADDR = dst;
  *DMA_LEN_ADDR = len;
  *DMA_OP_ADDR = DMA_OP_MEMCPY;
  while (*DMA_STAT_ADDR != 0)
    ; // Polling until DMA finished
}

int main(int argc, char *argv[]) {

  read_bmp(INPUT_BMP);
  printf("======================================\n");
  printf("\t  Reading from array\n");
  printf("======================================\n");
  printf(" input_data_offset\t= %d\n", input_data_offset);
  printf(" width\t\t\t= %d\n", width);
  printf(" height\t\t\t= %d\n", height);
  printf(" bits_per_pixel\t\t= %d\n", bits_per_pixel);
  printf("======================================\n");

  // Padding the Image
  int pad = 2;
  int padded_width = width + 2 * pad;
  int padded_height = height + 2 * pad;
  unsigned char *padded_bitmap =
      (unsigned char *)calloc(padded_width * padded_height, 1);
  unsigned int t_sw_start = read_cycles();
  for (int y = 0; y < (int)height; ++y) {
    for (int x = 0; x < (int)width; ++x) {
      padded_bitmap[(y + pad) * padded_width + (x + pad)] =
          gray_bitmap[y * width + x];
    }
  }
  unsigned int t_sw_end = read_cycles();

  // Reset
  unsigned int t_rst_start = read_cycles();
  *(volatile uint32_t *)ACC_WIDTH_ADDR = (uint32_t)padded_width;
  *(volatile uint32_t *)ACC_HEIGHT_ADDR = (uint32_t)padded_height;
  unsigned int t_rst_end = read_cycles();

  // DMA write
  unsigned int t_hw_write_start = read_cycles();
  dma((uint32_t)padded_bitmap, ACC_INPUT_ADDR, padded_width * padded_height);
  unsigned int t_hw_write_end = read_cycles();

  // DMA read
  unsigned int t_hw_read_start = read_cycles();
  dma(ACC_RESULT_ADDR, (uint32_t)target_bitmap, width * height);
  unsigned int t_hw_read_end = read_cycles();

  write_bmp(OUTPUT_BMP);

  printf("\n");
  printf("========================================\n");
  printf("   Timing Statistics (RISC-V Cycles)    \n");
  printf("========================================\n");
  printf("Software Padding Time  : %u cycles\n", t_sw_end - t_sw_start);
  printf("Reset HW               : %u cycles\n", t_rst_end - t_rst_start);
  printf("DMA Write & HW(Filter) : %u cycles\n",
         t_hw_write_end - t_hw_write_start);
  printf("DMA Read               : %u cycles\n",
         t_hw_read_end - t_hw_read_start);
  printf("Total Time             : %u cycles\n", t_hw_read_end - t_sw_start);
  printf("========================================\n");

  free(gray_bitmap);
  free(target_bitmap);
  free(padded_bitmap);

  return 0;
}
