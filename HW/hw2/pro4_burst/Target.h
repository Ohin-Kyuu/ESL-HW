#ifndef SCHARR_H_
#define SCHARR_H_

#include "filter_def.h"
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
  std::vector<unsigned char> result_buf; // Output Buffer for Burst

  SC_HAS_PROCESS(Target);
  Target(sc_core::sc_module_name n, unsigned int w, unsigned int h);
  ~Target() = default;

  virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &payload,
                                             tlm::tlm_phase &phase,
                                             sc_core::sc_time &delay);

private:
  unsigned char do_median(int u, int v);
  int do_scharr(int u, int v);

  std::vector<std::vector<unsigned char>> med_buf; // Median Line Buffer
  std::vector<std::vector<unsigned char>> sch_buf; // Scharr Line Buffer

  unsigned int x = 0, y = 0; // index for Storing Line Buf
  unsigned int base_offset = 0;
  unsigned int width;
  unsigned int height;

  // PEQ thread
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_peq;
  void peq_process_thread();
};

#endif // !SCHARR_H_
