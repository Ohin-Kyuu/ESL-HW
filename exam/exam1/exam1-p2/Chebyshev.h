#ifndef CHEBYSHEV_H_
#define CHEBYSHEV_H_

#include "filter_def.h"
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

class Chebyshev : public sc_core::sc_module {
public:
  tlm_utils::simple_target_socket<Chebyshev> t_skt;

  SC_CTOR(Chebyshev);

private:
  void blocking_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay);

  // Shift Reg
  ff_t x_1 = 0, x_2 = 0, x_3 = 0, x_4 = 0;
  ff_t y_1 = 0, y_2 = 0, y_3 = 0;

  ff_t current_result = 0;
};

#endif
