#include "Median.h"
#include "filter_def.h"
#include <cmath>
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

void Median::do_filter() {
  while (true) {
    sc_uint<8> row_med[3];
    for (unsigned int i = 0; i < MASK_Y; ++i) {
      sc_uint<8> a = i_g.read();
      sc_uint<8> b = i_g.read();
      sc_uint<8> c = i_g.read();
      row_med[i] = med3(a, b, c);
    }

    sc_uint<8> med = med3(row_med[0], row_med[1], row_med[2]);
    o_result.write(med);
    wait();
  }
}
