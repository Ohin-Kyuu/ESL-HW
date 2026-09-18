#include <iostream>
#include <systemc>

#include "HammingMatch.h"
#include "Testbench.h"

using namespace std;

// #define CLOCK_PERIOD 10

int sc_main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <train.txt> <query.txt>" << endl;
    cout << "Output: results.csv" << endl;
    return 1;
  }
  // Create modules and signals
  Testbench tb("tb");
  HammingMatch hm("hm");

  // CLK & RST
  sc_clock clk("clk", CLOCK_PERIOD, SC_NS);
  sc_signal<bool> rst("rst");

  // Create FIFO channels
  sc_fifo<sc_uint<32>> fifo_train(32);
  sc_fifo<sc_uint<32>> fifo_query(8);
  sc_fifo<sc_uint<8>> fifo_best_idx(4);
  sc_fifo<sc_uint<9>> fifo_min_dist(4);

  // Connect FIFO channels with modules
  // CLK & RST
  tb.i_clk(clk);
  tb.o_rst(rst);
  hm.i_clk(clk);
  hm.i_rst(rst);

  // Data FIFO
  tb.o_train(fifo_train);
  tb.o_query(fifo_query);
  hm.i_train(fifo_train);
  hm.i_query(fifo_query);
  hm.o_best_idx(fifo_best_idx);
  hm.o_min_dist(fifo_min_dist);
  tb.i_best_idx(fifo_best_idx);
  tb.i_min_dist(fifo_min_dist);

  // Load Descriptors
  if (tb.read_train(argv[1]) != 0)
    return 1;
  if (tb.read_query(argv[2]) != 0)
    return 1;

  sc_start();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;

  tb.write_results("results.csv");
  tb.verify();

  return 0;
}
