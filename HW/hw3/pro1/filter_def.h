#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

const int MASK_N = 2;
const int MASK_X = 3;
const int MASK_Y = 3;

#ifndef WIDTH
#define WIDTH 256
#endif
#ifndef HEIGHT
#define HEIGHT 256
#endif

const int IMG_X = WIDTH;
const int IMG_Y = HEIGHT;

const int PAD = 2;                       // 2 stages of 3x3 = 1+1 row/col each
const int STREAM_W = IMG_X + 2 * PAD;
const int STREAM_H = IMG_Y + 2 * PAD;

#endif
