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

    // Start (9 pixels)
    for (int v = 0; v < MASK_Y; ++v) {
      for (int u = 0; u < MASK_X; ++u) {
        buffer[v][u] = i_g.read();
      }
      row_med[v] = med3(buffer[v][0], buffer[v][1], buffer[v][2]);
    }

    sc_uint<8> med = med3(row_med[0], row_med[1], row_med[2]);
    o_result.write(med);
    wait();

    // Row Buf (2 Step  3+3 pixels)
    for (int step = 0; step < 2; ++step) {
      row_med[0] = row_med[1];
      row_med[1] = row_med[2];

      for (int u = 0; u < 3; ++u) {
        buffer[0][u] = buffer[1][u];
        buffer[1][u] = buffer[2][u];
        buffer[2][u] = i_g.read();
      }
      row_med[2] = med3(buffer[2][0], buffer[2][1], buffer[2][2]);

      sc_uint<8> med = med3(row_med[0], row_med[1], row_med[2]);
      o_result.write(med);
      wait();
    }
  } // Here Median Update A col to Scharr as 1 Col Buf when finish
}
