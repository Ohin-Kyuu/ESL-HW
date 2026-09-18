#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

#define CLOCK_PERIOD 10

const int MASK_N = 2;
const int MASK_X = 3;
const int MASK_Y = 3;

// MMIO
const int MOD_WIDTH_ADDR = 0x00000000;
const int MOD_HEIGHT_ADDR = 0x00000004;
const int MOD_INPUT_ADDR = 0x00400000;
const int MOD_RESULT_ADDR = 0x00800000;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

// Scharr mask
const int mask[MASK_N][MASK_Y][MASK_X] = {
    {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}}, // G_x
    {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}}  // G_y
};

// Median logic
inline unsigned char med3(unsigned char a, unsigned char b, unsigned char c) {
  if ((a <= b && b <= c) || (c <= b && b <= a))
    return b;
  if ((b <= a && a <= c) || (c <= a && a <= b))
    return a;
  return c;
}

#endif
