#include <iostream>
#include <string>
#include <sys/time.h>
#include <sysc/communication/sc_fifo.h>
#include <sysc/datatypes/int/sc_uint.h>
#include <sysc/kernel/sc_simcontext.h>

#include "Median.h"
#include "Scharr.h"
#include "Testbench.h"

using namespace std;

int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }

  // Create modules and signals
  Testbench tb("tb");
  Median median("median");
  Scharr scharr("scharr");
  sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  sc_signal<bool> rst("rst");

  // Create FIFO channels
  sc_fifo<sc_uint<8>> in;
  sc_fifo<sc_uint<8>> med;
  sc_fifo<int> result;

  // Connect FIFO channels with modules
  // CLK & RST
  tb.i_clk(clk);
  tb.o_rst(rst);
  median.i_clk(clk);
  median.i_rst(rst);
  scharr.i_clk(clk);
  scharr.i_rst(rst);
  // Data FIFO
  tb.o_g(in);
  median.i_g(in);
  median.o_result(med);
  scharr.i_g(med);
  scharr.o_result(result);
  tb.i_result(result);

  tb.read_bmp(argv[1]);
  sc_start();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  tb.write_bmp(argv[2]);

  return 0;
}
