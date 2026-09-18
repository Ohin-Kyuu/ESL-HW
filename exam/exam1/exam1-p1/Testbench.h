#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include "filter_def.h"
#include <systemc>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class Testbench : public sc_module {
public:
  sc_in_clk i_clk;
  sc_out<bool> o_rst;

  sc_out<ff_t> o_xn;
  sc_out<bool> o_xn_vld;
  sc_in<bool> i_xn_rdy;

  sc_in<ff_t> i_yn;
  sc_in<bool> i_yn_vld;
  sc_out<bool> o_yn_rdy;

  std::string infile_name;
  std::string outfile_name;

  SC_CTOR(Testbench);

private:
  void source();
  void sink();
};

#endif // !TESTBENCH_H_
