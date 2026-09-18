#include "Testbench.h"
#include <fstream>
#include <iostream>

Testbench::Testbench(sc_module_name n) : sc_module(n) {
  SC_THREAD(source);
  sensitive << i_clk.pos();
  dont_initialize();

  SC_THREAD(sink);
  sensitive << i_clk.pos();
  dont_initialize();
}

void Testbench::source() {
  o_xn_vld.write(false);
  o_rst.write(false);
  wait();
  o_rst.write(true);
  wait();

  std::ifstream fin(infile_name.c_str());
  if (!fin.is_open()) {
    std::cerr << "Error: Could not open input file " << infile_name
              << std::endl;
    sc_stop();
    return;
  }

  double val;
  for (int i = 0; i < SIGNAL_LENGTH; ++i) {
    if (fin >> val) {
      o_xn.write((ff_t)val);
    } else {
      o_xn.write(0);
    }

    // rdy/vld
    o_xn_vld.write(true);
    do {
      wait();
    } while (i_xn_rdy.read() == false);
    o_xn_vld.write(false);
  }
  fin.close();
}

void Testbench::sink() {
  // wait for reset
  o_yn_rdy.write(false);
  wait();

  std::ofstream fout(outfile_name.c_str());
  if (!fout.is_open()) {
    std::cerr << "Error: Could not open output file " << outfile_name
              << std::endl;
    sc_stop();
    return;
  }

  ff_t val;
  for (int i = 0; i < SIGNAL_LENGTH; ++i) {
    o_yn_rdy.write(true);
    do {
      wait();
    } while (i_yn_vld.read() == false);
    val = i_yn.read();
    o_yn_rdy.write(false);

    fout << val.to_double() << std::endl;
  }
  fout.close();

  std::cout << "Execution finished." << std::endl;
  sc_stop();
}
