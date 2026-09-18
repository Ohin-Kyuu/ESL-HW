#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class Testbench : public sc_module {
public:
  sc_in_clk i_clk;
  sc_out<bool> o_rst;

  sc_fifo_in<int> i_result;
  sc_fifo_out<sc_uint<8>> o_g;

  SC_CTOR(Testbench);

  int read_bmp(const std::string &infile_name);
  int write_bmp(const std::string &outfile_name);

private:
  void do_modules();

  unsigned int width;
  unsigned int height;

  unsigned short bits_per_pixel;
  unsigned int input_data_offset;

  unsigned char *gray_bitmap;   // internal grayscale input image
  unsigned char *target_bitmap; // grayscale output image
};

#endif // !TESTBENCH_H_
