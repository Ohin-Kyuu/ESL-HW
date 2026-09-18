#include "Scharr.h"
#include "filter_def.h"
#include <cmath>

Scharr::Scharr(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

// scharr mask
const int mask[MASK_N][MASK_Y][MASK_X] = {
    {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}}, // G_x
    {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}}  // G_y
};

void Scharr::do_filter() {
  while (true) {
    for (unsigned int i = 0; i < MASK_N; ++i) {
      val[i] = 0;
    }

    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
        sc_uint<8> grey = i_g.read();
        for (unsigned int i = 0; i < MASK_N; ++i) {
          val[i] += grey * mask[i][v][u];
        }
      }
    }

    double total = 0.0;
    for (unsigned int i = 0; i < MASK_N; ++i) {
      total += (double)val[i] * (double)val[i];
    }

    double grad = std::sqrt(total);
    int result = (int)(std::round(grad));

    // clip
    if (result > 255)
      result = 255;
    if (result < 0)
      result = 0;

    o_result.write(result);

    wait(); // emulate module delay
  }
}
