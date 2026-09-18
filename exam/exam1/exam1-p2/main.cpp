#include "Chebyshev.h"
#include "Testbench.h"
#include <iostream>
#include <systemc>

int sc_main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input_file> <output_file>"
              << std::endl;
    return -1;
  }

  // set the quantum_time for quantum keeper here !!!
  sc_core::sc_time quantum_time(1000, sc_core::SC_NS);

  Testbench tb("tb", quantum_time);
  Chebyshev filter("Chebyshev");

  // TLM Socket
  tb.initiator.i_skt(filter.t_skt);

  tb.infile_name = argv[1];
  tb.outfile_name = argv[2];

  sc_start();

  std::cout << "Quantum Limit applied: " << quantum_time << std::endl;
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;

  return 0;
}
