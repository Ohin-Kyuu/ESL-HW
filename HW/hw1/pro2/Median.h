#ifndef MEDIAN_H_
#define MEDIAN_H_

#include "filter_def.h"
#include <sysc/datatypes/int/sc_uint.h>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class Median : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;

  sc_fifo_in<sc_uint<8>> i_g;
  sc_fifo_out<sc_uint<8>> o_result;

  SC_CTOR(Median);
  ~Median() = default;

private:
  void do_filter();
  sc_uint<8> buffer[MASK_Y][MASK_X];
};

#endif // !MEDIAN_H_
