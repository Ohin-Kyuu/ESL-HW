#include <cstdint>

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

// Target ACC
static const uint32_t ACC_BASE_ADDR = 0x73000000;
static const uint32_t ACC_WIDTH_ADDR = ACC_BASE_ADDR + 0x00000000;
static const uint32_t ACC_HEIGHT_ADDR = ACC_BASE_ADDR + 0x00000004;
static const uint32_t ACC_INPUT_ADDR = ACC_BASE_ADDR + 0x00400000;  // 4 MB
static const uint32_t ACC_RESULT_ADDR = ACC_BASE_ADDR + 0x00800000; // 8 MB

// DMA
static volatile uint32_t *const DMA_SRC_ADDR = (uint32_t *const)0x70000000;
static volatile uint32_t *const DMA_DST_ADDR = (uint32_t *const)0x70000004;
static volatile uint32_t *const DMA_LEN_ADDR = (uint32_t *const)0x70000008;
static volatile uint32_t *const DMA_OP_ADDR = (uint32_t *const)0x7000000C;
static volatile uint32_t *const DMA_STAT_ADDR = (uint32_t *const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};
