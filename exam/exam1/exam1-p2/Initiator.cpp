#include "Initiator.h"

Initiator::Initiator(sc_module_name n, sc_time quantum_limit)
    : sc_module(n), i_skt("i_skt") {
  m_qk.set_global_quantum(quantum_limit);
  m_qk.reset();
}

int Initiator::read_from_socket(unsigned long int addr, unsigned char mask[],
                                unsigned char rdata[], int dataLen) {
  trans.set_read();
  trans.set_address((sc_dt::uint64)addr);
  trans.set_byte_enable_length((const unsigned int)dataLen);
  trans.set_byte_enable_ptr(mask);
  trans.set_data_length((const unsigned int)dataLen);
  trans.set_data_ptr(rdata);
  do_trans(trans);
  return trans.is_response_ok() ? 0 : -1;
}

int Initiator::write_to_socket(unsigned long int addr, unsigned char mask[],
                               unsigned char wdata[], int dataLen) {
  trans.set_write();
  trans.set_address((sc_dt::uint64)addr);
  trans.set_byte_enable_length((const unsigned int)dataLen);
  trans.set_byte_enable_ptr(mask);
  trans.set_data_length((const unsigned int)dataLen);
  trans.set_data_ptr(wdata);
  do_trans(trans);
  return trans.is_response_ok() ? 0 : -1;
}

void Initiator::do_trans(tlm::tlm_generic_payload &trans) {
  sc_core::sc_time dummyDelay = m_qk.get_local_time();

  i_skt->b_transport(trans, dummyDelay);

  m_qk.set(dummyDelay);
  if (m_qk.need_sync()) {
    m_qk.sync();
  }
}
