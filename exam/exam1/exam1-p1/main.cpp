#include <iostream>

#include "Chebyshev.h"
#include "Testbench.h"

using namespace std;

int sc_main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_file> <output_file>"
              << std::endl;
    return -1;
  }

  Testbench tb("tb");
  Chebyshev filter("Chebyshev");
  sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  sc_signal<bool> rst("rst");

  // rdy/vld signal
  sc_signal<ff_t> xn_sig;
  sc_signal<bool> xn_vld_sig;
  sc_signal<bool> xn_rdy_sig;

  sc_signal<ff_t> yn_sig;
  sc_signal<bool> yn_vld_sig;
  sc_signal<bool> yn_rdy_sig;

  // CLK & RST
  tb.i_clk(clk);
  tb.o_rst(rst);
  filter.i_clk(clk);
  filter.i_rst(rst);

  // Input Channel (Testbench -> Filter)
  tb.o_xn(xn_sig);
  tb.o_xn_vld(xn_vld_sig);
  tb.i_xn_rdy(xn_rdy_sig);

  filter.i_xn(xn_sig);
  filter.i_xn_vld(xn_vld_sig);
  filter.o_xn_rdy(xn_rdy_sig);

  // Output Channel (Filter -> Testbench)
  filter.o_yn(yn_sig);
  filter.o_yn_vld(yn_vld_sig);
  filter.i_yn_rdy(yn_rdy_sig);

  tb.i_yn(yn_sig);
  tb.i_yn_vld(yn_vld_sig);
  tb.o_yn_rdy(yn_rdy_sig);

  tb.infile_name = argv[1];
  tb.outfile_name = argv[2];

  sc_start();

  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  return 0;
}
