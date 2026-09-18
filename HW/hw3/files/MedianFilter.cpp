#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "MedianFilter.h"

MedianFilter::MedianFilter(sc_module_name n) : sc_module(n) {
  SC_CTHREAD(do_filter, i_clk.pos());
  reset_signal_is(i_rst, 0);

#ifndef NATIVE_SYSTEMC
  i_grey.clk_rst(i_clk, i_rst);
  o_grey.clk_rst(i_clk, i_rst);
#endif
}

static inline sc_dt::sc_uint<8> med3(sc_dt::sc_uint<8> a,
                                     sc_dt::sc_uint<8> b,
                                     sc_dt::sc_uint<8> c) {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

sc_dt::sc_uint<8> MedianFilter::median(sc_dt::sc_uint<8> ws[3][3]) {
  HLS_DPO("median");
  HLS_LAT_LOG("median");

  sc_dt::sc_uint<8> row_med[3];
  HLS_FLATTEN(row_med);
  
  for (int i = 0; i < 3; ++i) {
    HLS_UNROLL("row_loop");
    row_med[i] = med3(ws[i][0], ws[i][1], ws[i][2]);
  }

  return med3(row_med[0], row_med[1], row_med[2]);
}

void MedianFilter::do_filter() {
  HLS_RESET_BLOCK(
    HLS_RESET_IN(i_grey);
    HLS_RESET_OUT(o_grey);
  )

  sc_dt::sc_uint<8> ws[3][3];
  HLS_FLATTEN(ws);
  
  GLB_START(i_grey);
  GLB_START(o_grey);

  while (!GLB_Y_DONE(i_grey)) {
    while (!GLB_X_DONE(i_grey)) {
       
      HLS_PIPELINE("median_loop");
      
      GLB_GET(i_grey, ws);
      sc_dt::sc_uint<8> out = median(ws);
      GLB_PUT(o_grey, out);
    }
    GLB_NEXT_Y(i_grey);
    GLB_NEXT_Y(o_grey);
  }

  GLB_END(i_grey);
  GLB_END(o_grey);
}
