#ifndef CHEBYSHEV_H_
#define CHEBYSHEV_H_

#include "filter_def.h"

using namespace sc_core;

class Chebyshev : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;

  // Input Channel
  sc_in<ff_t> i_xn;
  sc_in<bool> i_xn_vld;
  sc_out<bool> o_xn_rdy;

  // Output Channel
  sc_out<ff_t> o_yn;
  sc_out<bool> o_yn_vld;
  sc_in<bool> i_yn_rdy;

  SC_CTOR(Chebyshev);

private:
  void do_filter();

  ff_t x_1 = 0, x_2 = 0, x_3 = 0, x_4 = 0;
  ff_t y_1 = 0, y_2 = 0, y_3 = 0;
};

#endif
