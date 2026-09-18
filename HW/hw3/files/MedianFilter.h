#ifndef MEDIAN_FILTER_H_
#define MEDIAN_FILTER_H_

#include <systemc>
using namespace sc_core;

#include "hls_def.h"
#include "filter_def.h"

class MedianFilter : public sc_module {
public:
  sc_in_clk   i_clk;
  sc_in<bool> i_rst;

  GLB_IN  i_grey;
  GLB_OUT o_grey;

  SC_HAS_PROCESS(MedianFilter);
  MedianFilter(sc_module_name n);

private:
  void do_filter();
  sc_dt::sc_uint<8> median(sc_dt::sc_uint<8> ws[3][3]);
};

#endif
