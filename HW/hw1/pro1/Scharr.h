#ifndef SCHARR_H_
#define SCHARR_H_

#include "filter_def.h"
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class Scharr : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;

  sc_fifo_in<sc_uint<8>> i_g;
  sc_fifo_out<int> o_result;

  SC_CTOR(Scharr);
  ~Scharr() = default;

private:
  void do_filter();
  int val[MASK_N];
};

#endif // !SCHARR_H_
