#include "Testbench.h"
#include <fstream>
#include <iostream>

Testbench::Testbench(sc_module_name n, sc_time quantum_limit)
    : sc_module(n), initiator("initiator", quantum_limit) {
  SC_THREAD(do_modules);
}

void Testbench::do_modules() {
  std::ifstream fin(infile_name.c_str());
  std::ofstream fout(outfile_name.c_str());

  if (!fin.is_open() || !fout.is_open()) {
    std::cerr << "File Error!" << std::endl;
    sc_stop();
    return;
  }

  // TLM mask
  unsigned char mask[sizeof(ff_t)];
  memset(mask, 0xff, sizeof(ff_t)); // all byte-enable

  double val;
  ff_t val_in, val_out;

  for (int i = 0; i < SIGNAL_LENGTH; ++i) {
    if (fin >> val) {
      val_in = (ff_t)val;
    } else {
      val_in = 0;
    }

    // Write
    initiator.write_to_socket(MOD_INPUT_ADDR, mask, (unsigned char *)&val_in,
                              sizeof(ff_t));

    // Read
    initiator.read_from_socket(MOD_RESULT_ADDR, mask, (unsigned char *)&val_out,
                               sizeof(ff_t));

    fout << val_out.to_double() << std::endl;
  }

  fin.close();
  fout.close();
  sc_stop();
}
