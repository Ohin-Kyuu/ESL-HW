#ifndef SCHARR_FILTER_H_
#define SCHARR_FILTER_H_

#include <systemc>
using namespace sc_core;

#include "hls_def.h"
#include "filter_def.h"

class ScharrFilter : public sc_module {
public:
  sc_in_clk   i_clk;
  sc_in<bool> i_rst;

  GLB_IN                          i_grey;
  HLS_OUT(sc_dt::sc_uint<32>)     o_result;

  SC_HAS_PROCESS(ScharrFilter);
  ScharrFilter(sc_module_name n);

private:
  void do_filter();
  sc_dt::sc_uint<32> gradient(sc_dt::sc_uint<8> ws[3][3]);
};

#endif
