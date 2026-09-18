#include "Initiator.h"

using namespace std;

Initiator::Initiator(sc_module_name n) : sc_module(n), i_skt("i_skt") {
  i_skt.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
}

tlm::tlm_sync_enum Initiator::nb_transport_bw(tlm::tlm_generic_payload &trans,
                                              tlm::tlm_phase &phase,
                                              sc_core::sc_time &delay) {
  if (phase == tlm::BEGIN_RESP) {
    end_request_event.notify(delay);
    return tlm::TLM_COMPLETED;
  }
  return tlm::TLM_ACCEPTED;
}

int Initiator::read_from_socket(unsigned long int addr, unsigned char mask[],
                                unsigned char rdata[], int dataLen) {
  // Reset Status
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  // Set up the payload fields. Assume everything is 4 bytes.
  trans.set_read();
  trans.set_address((sc_dt::uint64)addr);

  trans.set_byte_enable_length((const unsigned int)dataLen);
  trans.set_byte_enable_ptr((unsigned char *)mask);

  trans.set_data_length((const unsigned int)dataLen);
  trans.set_data_ptr((unsigned char *)rdata);

  // Transport.
  do_trans(trans);

  /* For now just simple non-zero return code on error */
  return trans.is_response_ok() ? 0 : -1;

} // read_from_socket()

int Initiator::write_to_socket(unsigned long int addr, unsigned char mask[],
                               unsigned char wdata[], int dataLen) {
  // Reset Status
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  // Set up the payload fields. Assume everything is 4 bytes.
  trans.set_write();
  trans.set_address((sc_dt::uint64)addr);

  trans.set_byte_enable_length((const unsigned int)dataLen);
  trans.set_byte_enable_ptr((unsigned char *)mask);

  trans.set_data_length((const unsigned int)dataLen);
  trans.set_data_ptr((unsigned char *)wdata);

  // Transport.
  do_trans(trans);

  /* For now just simple non-zero return code on error */
  return trans.is_response_ok() ? 0 : -1;

} // writeUpcall()

void Initiator::do_trans(tlm::tlm_generic_payload &trans) {
  sc_core::sc_time dummyDelay = sc_core::SC_ZERO_TIME;

  // Start Request
  // cout << "[Initiator] Send Request at " << sc_time_stamp() << " addr: 0x"
  //      << hex << trans.get_address() << dec << endl;
  tlm::tlm_phase phase = tlm::BEGIN_REQ;

  tlm::tlm_sync_enum status = i_skt->nb_transport_fw(trans, phase, dummyDelay);
  if (status == tlm::TLM_ACCEPTED || status == tlm::TLM_UPDATED) {
    wait(end_request_event);
    // cout << "[Initiator] Received Response (Wake up) at " << sc_time_stamp()
    // << endl;
  } else if (status == tlm::TLM_COMPLETED) {
    wait(dummyDelay);
  }

} // do_trans()
