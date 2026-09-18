#include "Median.h"
#include "filter_def.h"
#include <sysc/datatypes/int/sc_uint.h>

inline sc_uint<8> med3(sc_uint<8> a, sc_uint<8> b, sc_uint<8> c) {
  if ((a <= b && b <= c) || (c <= b && b <= a))
    return b;
  if ((b <= a && a <= c) || (c <= a && a <= b))
    return a;
  return c;
}

Median::Median(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

sc_uint<8> Median::compute(int v, int u) {
  sc_uint<8> row_med[MASK_Y];
  for (int i = 0; i < MASK_Y; ++i) {
    row_med[i] = med3(med_buf[(v + i) % 3][u + 0], med_buf[(v + i) % 3][u + 1],
                      med_buf[(v + i) % 3][u + 2]);
  }
  return med3(row_med[0], row_med[1], row_med[2]);
}

void Median::do_filter() {
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < STREAM_W; ++j)
      med_buf[i][j] = 0;

  int y = 0, x = 0;

  while (true) {
    sc_uint<8> in_pix = i_g.read();
    med_buf[y % 3][x] = in_pix;

    int v = (y - 2 + 3) % 3;
    int u = x - 2;

    sc_uint<8> result = 0;
    if (y >= 2 && x >= 2) {
      result = compute(v, u);
    }
    o_result.write(result);
    wait();

    x++;
    if (x == STREAM_W) {
      x = 0;
      y++;
    }
  }
}
