#include "Chebyshev.h"

Chebyshev::Chebyshev(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
}

void Chebyshev::do_filter() {
  // Reset
  o_xn_rdy.write(false);
  o_yn_vld.write(false);
  wait();

  while (true) {
    // Input Results when valid
    o_xn_rdy.write(true);
    do {
      wait();
    } while (i_xn_vld.read() == false);
    ff_t x_in = i_xn.read();
    o_xn_rdy.write(false);

    // MAC
    ff_t y_out = 0;

    y_out += 0.88244379 * x_1;
    wait();
    y_out -= 2.63978216 * x_2;
    wait();
    y_out += 2.63978216 * x_3;
    wait();
    y_out -= 0.88244379 * x_4;
    wait();

    y_out += 2.7421805 * y_1;
    wait();
    y_out -= 2.52356443 * y_2;
    wait();
    y_out += 0.77870694 * y_3;
    wait();

    // Output Results
    o_yn.write(y_out);
    o_yn_vld.write(true);
    do {
      wait();
    } while (i_yn_rdy.read() == false);
    o_yn_vld.write(false);

    x_4 = x_3;
    x_3 = x_2;
    x_2 = x_1;
    x_1 = x_in;

    y_3 = y_2;
    y_2 = y_1;
    y_1 = y_out;
  }
}
