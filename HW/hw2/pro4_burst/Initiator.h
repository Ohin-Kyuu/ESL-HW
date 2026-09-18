#ifndef INITIATOR_H_
#define INITIATOR_H_
#include <systemc>
#include <tlm_core/tlm_2/tlm_2_interfaces/tlm_fw_bw_ifs.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
using namespace sc_core;

#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"

class Initiator : public sc_module {
public:
  tlm_utils::simple_initiator_socket<Initiator> i_skt;

  SC_CTOR(Initiator);

  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &trans,
                                             tlm::tlm_phase &phase,
                                             sc_core::sc_time &delay);

  // Wait for end
  sc_core::sc_event end_request_event;

  int read_from_socket(unsigned long int addr, unsigned char mask[],
                       unsigned char rdata[], int dataLen);

  int write_to_socket(unsigned long int addr, unsigned char mask[],
                      unsigned char wdata[], int dataLen);

  void do_trans(tlm::tlm_generic_payload &trans);
  tlm::tlm_generic_payload trans;
};
#endif
