#include <iostream>
using namespace std;

// Wall Clock Time Measurement
#include <sys/time.h>

#include "SimpleBus.h"
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

  SimpleBus<1, 1> bus("Bus");
  bus.setDecode(0, MOD_MM_BASE, MOD_MM_BASE + MOD_MM_SIZE - 1);

  Testbench tb("tb");
  tb.read_bmp(argv[1]);

  Target target("target", tb.width, tb.height);
  tb.initiator.i_skt(bus.t_skt[0]); // Testbench -> Bus
  bus.i_skt[0](target.t_skt);       // Bus -> Target

  sc_start();
  bus.report();
  std::cout << "Simulated time == " << sc_core::sc_time_stamp() << std::endl;
  tb.write_bmp(argv[2]);

  return 0;
}
