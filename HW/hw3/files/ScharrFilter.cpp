#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "ScharrFilter.h"

static const int scharr_mask[MASK_N][MASK_Y][MASK_X] = {
    {{-3,   0,  3}, {-10, 0, 10}, {-3,   0,  3}},  // Gx
    {{-3, -10, -3}, {  0, 0,  0}, { 3,  10,  3}}   // Gy
};

inline int isqrt(int x) {
  if (x <= 0) return 0;
  int res = 0;
  int bit = 1 << 30; 
  
  for (int i = 0; i < 16; ++i) {
    HLS_UNROLL("sqrt_loop");
    if (x >= res + bit) {
      x -= res + bit;
      res = (res >> 1) + bit;
    } else {
      res >>= 1;
    }
    bit >>= 2;
  }
  return res;
}

// inline int isqrt(int x) {
//   if (x <= 0) return 0;
//   int res = 0;
//   int bit = 1 << 30;
//   while (bit > x) bit >>= 2;
//   while (bit != 0) {
//     if (x >= res + bit) {
//       x -= res + bit;
//       res = (res >> 1) + bit;
//     } else {
//       res >>= 1;
//     }
//     bit >>= 2;
//   }
//   return res;
// }

ScharrFilter::ScharrFilter(sc_module_name n) : sc_module(n) {
  SC_CTHREAD(do_filter, i_clk.pos());
  reset_signal_is(i_rst, 0);

#ifndef NATIVE_SYSTEMC
  i_grey.clk_rst(i_clk, i_rst);
  o_result.clk_rst(i_clk, i_rst);
#endif
}

sc_dt::sc_uint<32> ScharrFilter::gradient(sc_dt::sc_uint<8> ws[3][3]) {
  int val[MASK_N];
  HLS_FLATTEN(val);
  HLS_DPO("scharr_gradient");
  HLS_LAT_LOG("scharr_gradient");

  for (int i = 0; i < MASK_N; ++i) {
    HLS_UNROLL("grainit_loop");
    val[i] = 0;
  }

  for (int v = 0; v < 3; ++v) {
    for (int u = 0; u < 3; ++u) {
      int grey = (int)ws[v][u];
      for (int i = 0; i < MASK_N; ++i) {  
        HLS_UNROLL("gradient_loop");
        val[i] += grey * scharr_mask[i][v][u];
      }
    }
  }

  int grad = isqrt(val[0] * val[0] + val[1] * val[1]);
  int clamped = (grad > 255) ? 255 : grad;

  return (sc_dt::sc_uint<32>)clamped;
}

void ScharrFilter::do_filter() {
  HLS_RESET_BLOCK(
    HLS_RESET_IN(i_grey);
    HLS_RESET_OUT(o_result);
  )

  sc_dt::sc_uint<8> ws[3][3];
  HLS_FLATTEN(ws);
  GLB_START(i_grey);

  while (!GLB_Y_DONE(i_grey)) {
    while (!GLB_X_DONE(i_grey)) {
     
      HLS_PIPELINE("scharr_loop");

      GLB_GET(i_grey, ws);
      sc_dt::sc_uint<32> out = gradient(ws);
      HLS_WRITE(o_result, out);
    }
    GLB_NEXT_Y(i_grey);
  }

  GLB_END(i_grey);
}
