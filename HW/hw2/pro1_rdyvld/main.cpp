#include <iostream>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

#include "Target.h"
#include "Testbench.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {
  if ((argc < 3) || (argc > 4)) {
    cout << "No arguments for the executable : " << argv[0] << endl;
    cout << "Usage : >" << argv[0] << " in_image_file_name out_image_file_name"
         << endl;
    return 0;
  }
  Testbench tb("tb");
  tb.read_bmp(argv[1]);

  Target target("target", tb.width, tb.height);
  tb.initiator.i_skt(target.t_skt);

  // Handshaking Protocal
  sc_signal<bool> vld;
  sc_signal<bool> rdy;

  tb.initiator.vld(vld);
  tb.initiator.rdy(rdy);

  target.vld(vld);
  target.rdy(rdy);

  sc_start();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  tb.write_bmp(argv[2]);

  return 0;
}
