#include "System.h"
#include <iostream>
#include <systemc>

#ifndef NATIVE_SYSTEMC
#include "esc.h"
#endif

using namespace std;

System *sys = NULL;

#ifndef NATIVE_SYSTEMC
extern void esc_elaborate() {
  sys = new System("sys", esc_argv(1), esc_argv(2));
}
extern void esc_cleanup() { delete sys; }
#endif

int sc_main(int argc, char **argv) {
  if (argc < 3) {
    cout << "Usage : " << argv[0] << " <train.txt> <query.txt>" << endl;
    return 0;
  }

#ifndef NATIVE_SYSTEMC
  esc_initialize(argc, argv);
  esc_elaborate();
#else
  sys = new System("sys", argv[1], argv[2]);
#endif

  sc_start();

#ifndef NATIVE_SYSTEMC
  esc_cleanup();
#else
  delete sys;
#endif

  return 0;
}
