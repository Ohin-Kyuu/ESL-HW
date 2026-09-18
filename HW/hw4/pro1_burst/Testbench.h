#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <string>
#include <systemc>

#include "Initiator.h"
#include "filter_def.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class Testbench : public sc_module {
public:
  Initiator initiator;

  SC_CTOR(Testbench);
  ~Testbench() = default;

  int read_bmp(const string &infile_name);
  int write_bmp(const string &outfile_name);

  unsigned int width;
  unsigned int height;

private:
  void do_modules();

  unsigned short bits_per_pixel;
  unsigned int input_data_offset;

  unsigned char *gray_bitmap;   // internal grayscale input image
  unsigned char *target_bitmap; // grayscale output image
};

#endif // !TESTBENCH_H_
