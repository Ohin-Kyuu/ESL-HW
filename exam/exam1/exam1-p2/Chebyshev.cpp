#include "Chebyshev.h"

using namespace sc_core;

Chebyshev::Chebyshev(sc_module_name n) : sc_module(n), t_skt("t_skt") {
  t_skt.register_b_transport(this, &Chebyshev::blocking_transport);
}

void Chebyshev::blocking_transport(tlm::tlm_generic_payload &payload,
                                   sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  unsigned char *data_ptr = payload.get_data_ptr();

  ff_t *payload_data = reinterpret_cast<ff_t *>(data_ptr);

  if (payload.get_command() == tlm::TLM_WRITE_COMMAND) {
    if (addr == MOD_INPUT_ADDR) {
      ff_t x_in = *payload_data;

      // Read (1) + MAC(7) = 8 cycles
      delay += sc_time(8 * CLOCK_PERIOD, SC_NS);

      // MAC
      ff_t y_out = 0;
      y_out += (ff_t)0.88244379 * x_1;
      y_out -= (ff_t)2.63978216 * x_2;
      y_out += (ff_t)2.63978216 * x_3;
      y_out -= (ff_t)0.88244379 * x_4;
      y_out += (ff_t)2.7421805 * y_1;
      y_out -= (ff_t)2.52356443 * y_2;
      y_out += (ff_t)0.77870694 * y_3;

      x_4 = x_3;
      x_3 = x_2;
      x_2 = x_1;
      x_1 = x_in;
      y_3 = y_2;
      y_2 = y_1;
      y_1 = y_out;

      current_result = y_out;
    } else {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }
  } else if (payload.get_command() == tlm::TLM_READ_COMMAND) {
    if (addr == MOD_RESULT_ADDR) {
      *payload_data = current_result;

      delay += sc_time(1 * CLOCK_PERIOD, SC_NS);
    } else {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE);
}
