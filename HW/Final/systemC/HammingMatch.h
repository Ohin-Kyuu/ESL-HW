#ifndef HAMMINGMATCH_H_
#define HAMMINGMATCH_H_

// #include "filter_def.h"
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class HammingMatch : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;

  // Descriptors: 256-bit = 32-bit x 8 words
  sc_fifo_in<sc_uint<32>> i_train;
  sc_fifo_in<sc_uint<32>> i_query;

  // Results
  sc_fifo_out<sc_uint<8>> o_best_idx;
  sc_fifo_out<sc_uint<9>> o_min_dist;

  SC_CTOR(HammingMatch);
  ~HammingMatch() = default;

private:
  void do_match();

  sc_uint<32> qword[8];
  sc_uint<32> tword[8]; // No Memory Bank
  // sc_uint<32> train_db[256][8]; // Memory Bank: [256 descriptors][8 words]
};

#endif // !HAMMINGMATCH_H_
