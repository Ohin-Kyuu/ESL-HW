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
  // At (0,0), Need 9 pixels first, after we use Buffer
  for (unsigned int u = 0; u < MASK_X; ++u) {
    for (unsigned int v = 0; v < MASK_Y; ++v) {
      buffer[v][u] = i_g.read();
    }
  }

  while (true) {
    for (unsigned int i = 0; i < MASK_N; ++i) {
      val[i] = 0;
    }

    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
        for (unsigned int i = 0; i < MASK_N; ++i) {
          val[i] += buffer[v][u] * mask[i][v][u];
        }
      }
    }

    double total = 0.0;
    for (unsigned int i = 0; i < MASK_N; ++i)
      total += (double)val[i] * (double)val[i];

    double grad = std::sqrt(total);
    int result = (int)(std::round(grad));

    // clip
    if (result > 255)
      result = 255;
    if (result < 0)
      result = 0;

    o_result.write(result);
    wait();

    // Update Col Buf from Median
    for (unsigned int v = 0; v < MASK_Y; ++v) {
      buffer[v][0] = buffer[v][1];
      buffer[v][1] = buffer[v][2];
      buffer[v][2] = i_g.read();
    }
  }
}
