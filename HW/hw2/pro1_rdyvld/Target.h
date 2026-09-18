#ifndef SCHARR_H_
#define SCHARR_H_

#include "filter_def.h"
#include <sysc/kernel/sc_module.h>
#include <systemc>
#include <vector>

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

using namespace sc_core;
using namespace sc_dt;

class Target : public sc_module {
public:
  tlm_utils::simple_target_socket<Target> t_skt;

  unsigned char gry;
  int result;

  SC_HAS_PROCESS(Target);
  // Handshaking
  sc_in<bool> vld;
  sc_out<bool> rdy;

  Target(sc_core::sc_module_name n, unsigned int w, unsigned int h);
  ~Target() = default;

private:
  unsigned char do_median(int u, int v);
  int do_scharr(int u, int v);

  std::vector<std::vector<unsigned char>> med_buf; // Median Line Buffer
  std::vector<std::vector<unsigned char>> sch_buf; // Scharr Line Buffer

  unsigned int x = 0, y = 0; // index for Storing Line Buf
  unsigned int base_offset = 0;
  unsigned int width;
  unsigned int height;

  void blocking_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay);

  void rdyvld_thread();
};

#endif // !SCHARR_H_
