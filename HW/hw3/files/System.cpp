#include "System.h"

System::System(sc_module_name n, std::string input_bmp,
               std::string output_bmp)
    : sc_module(n),
      tb("tb"),
      median_filter("median_filter"),
      scharr_filter("scharr_filter"),
      clk("clk", CLOCK_PERIOD, SC_NS),
      rst("rst"),
      _output_bmp(output_bmp) {
  tb.i_clk(clk);
  tb.o_rst(rst);

  median_filter.i_clk(clk);
  median_filter.i_rst(rst);

  scharr_filter.i_clk(clk);
  scharr_filter.i_rst(rst);

  tb.o_g(grey_ch);
  median_filter.i_grey(grey_ch);

  median_filter.o_grey(median_ch);
  scharr_filter.i_grey(median_ch);

  scharr_filter.o_result(result_ch);
  tb.i_result(result_ch);

  tb.read_bmp(input_bmp);
}

System::~System() {
  tb.write_bmp(_output_bmp);
}
