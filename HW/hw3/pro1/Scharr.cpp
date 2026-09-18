#include "Scharr.h"
#include "filter_def.h"
#include <cmath>
#include <sysc/datatypes/int/sc_uint.h>

const int mask[MASK_N][MASK_Y][MASK_X] = {
    {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}}, // G_x
    {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}}  // G_y
};

Scharr::Scharr(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

int Scharr::compute(int v, int u) {
  int val[MASK_N] = {0};
  for (int i = 0; i < MASK_Y; ++i) {
    for (int j = 0; j < MASK_X; ++j) {
      sc_uint<8> grey = sch_buf[(v + i) % 3][u + j];
      for (int k = 0; k < MASK_N; ++k) {
        val[k] += grey * mask[k][i][j];
      }
    }
  }
  double total = 0.0;
  for (int i = 0; i < MASK_N; ++i) {
    total += (double)val[i] * (double)val[i];
  }
  double grad = std::sqrt(total);
  int result = (int)(std::round(grad));
  if (result > 255)
    result = 255;
  if (result < 0)
    result = 0;
  return result;
}

void Scharr::do_filter() {
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < STREAM_W; ++j)
      sch_buf[i][j] = 0;

  int y = 0, x = 0;

  while (true) {
    sc_uint<8> pixel = i_g.read();
    sch_buf[y % 3][x] = pixel;

    int v = (y - 2 + 3) % 3;
    int u = x - 2;

    int result = 0;
    if (y >= 2 && x >= 2) {
      result = (sc_uint<8>)compute(v, u);
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
