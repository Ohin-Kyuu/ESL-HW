#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
#include <systemc>
using namespace sc_core;
using namespace sc_dt;

#include "hls_def.h"
#include "filter_def.h"

class Testbench : public sc_module {
public:
  sc_in_clk    i_clk;
  sc_out<bool> o_rst;

  GLB_TB_OUT                       o_g;
  HLS_TB_IN(sc_dt::sc_uint<32>)    i_result;

  SC_HAS_PROCESS(Testbench);
  Testbench(sc_module_name n);
  ~Testbench();

  int read_bmp(const std::string &infile_name);
  int write_bmp(const std::string &outfile_name);

private:
  void feed();
  void fetch();

  unsigned int   width;
  unsigned int   height;
  unsigned short bits_per_pixel;
  unsigned int   input_data_offset;

  unsigned char *gray_bitmap;
  unsigned char *target_bitmap;

  sc_time total_start_time;
  sc_time total_run_time;
};

#endif
