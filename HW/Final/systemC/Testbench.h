#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;

class Testbench : public sc_module {
public:
  sc_in_clk i_clk;
  sc_out<bool> o_rst;

  // Descriptors: 256-bit = 32-bit x 8 words
  sc_fifo_out<sc_uint<32>> o_train;
  sc_fifo_out<sc_uint<32>> o_query;

  // Results
  sc_fifo_in<sc_uint<8>> i_best_idx;
  sc_fifo_in<sc_uint<9>> i_min_dist;

  SC_CTOR(Testbench);
  ~Testbench() = default;

  int read_train(const std::string &filename);
  int read_query(const std::string &filename);
  void write_results(const std::string &filename);
  void verify();

private:
  void do_tb();
  int load_descriptors(const std::string &filename, sc_uint<32> data[256][8]);

  // Descriptor Resgister Bank: [256 descriptors][8 words]
  sc_uint<32> train_db[256][8];
  sc_uint<32> query_db[256][8];

  // Output buffers
  sc_uint<8> best_idx_buf[256];
  sc_uint<9> min_dist_buf[256];
};

#endif // !TESTBENCH_H_
