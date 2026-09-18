#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <systemc>
using namespace sc_core;

#include "Testbench.h"
#include "hls_def.h"

#ifndef NATIVE_SYSTEMC
#include "MedianFilter_wrap.h"
#include "ScharrFilter_wrap.h"
#else
#include "MedianFilter.h"
#include "ScharrFilter.h"
#endif

class System : public sc_module {
public:
  SC_HAS_PROCESS(System);
  System(sc_module_name n, std::string input_bmp, std::string output_bmp);
  ~System();

private:
  Testbench tb;

#ifndef NATIVE_SYSTEMC
  MedianFilter_wrapper median_filter;
  ScharrFilter_wrapper scharr_filter;
#else
  MedianFilter median_filter;
  ScharrFilter scharr_filter;
#endif

  sc_clock        clk;
  sc_signal<bool> rst;

  GLB_CHAN                      grey_ch;    // Testbench    -> MedianFilter
  GLB_CHAN                      median_ch;  // MedianFilter -> ScharrFilter
  HLS_CHAN(sc_dt::sc_uint<32>)  result_ch;  // ScharrFilter -> Testbench

  std::string _output_bmp;
};

#endif
