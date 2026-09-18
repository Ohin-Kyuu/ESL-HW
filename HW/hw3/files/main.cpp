#include "System.h"
#include <iostream>
#include <string>
#include <systemc>

#ifndef NATIVE_SYSTEMC
#include "esc.h"
#endif

using namespace std;

System *sys = NULL;

char *in_bmp = NULL;
char *out_bmp = NULL;

#ifndef NATIVE_SYSTEMC
extern void esc_elaborate() {
  sys = new System("sys", esc_argv(1), esc_argv(2));
}
extern void esc_cleanup() { delete sys; }
#endif

int sc_main(int argc, char **argv) {
  if (argc < 3) {
    cout << "Usage : " << argv[0] << " <in_bmp> <out_bmp>" << endl;
    return 0;
  }

  in_bmp = argv[1];
  out_bmp = argv[2];

#ifndef NATIVE_SYSTEMC
  esc_initialize(argc, argv);
  esc_elaborate();
#else
  sys = new System("sys", in_bmp, out_bmp);
#endif

  sc_start();

#ifndef NATIVE_SYSTEMC
  esc_cleanup();
#else
  delete sys;
#endif

  // std::cout << "Simulation finished at " << sc_time_stamp() << std::endl;
  return 0;
}
