#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include "Initiator.h"
#include "filter_def.h"
#include <string>
#include <systemc>

using namespace sc_core;

class Testbench : public sc_module {
public:
  Initiator initiator;
  std::string infile_name;
  std::string outfile_name;

  SC_HAS_PROCESS(Testbench);
  Testbench(sc_module_name n, sc_time quantum_limit);

private:
  void do_modules();
};

#endif
