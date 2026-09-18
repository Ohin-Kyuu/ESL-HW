#include "Initiator.h"

Initiator::Initiator(sc_module_name n) : sc_module(n), i_skt("i_skt") {}

int Initiator::read_from_socket(unsigned long int addr, unsigned char mask[],
                                unsigned char rdata[], int dataLen) {
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

  // Handshaking
  vld.write(true);

  // using polling
  while (rdy.read() == false) {
    wait(CLOCK_PERIOD, SC_NS);
  }

  // if ready, then do the job
  sc_core::sc_time dummyDelay = sc_core::SC_ZERO_TIME;

  // Call the transport and wait for no time, which allows the thread to yield
  // and others to get a look in!

  i_skt->b_transport(trans, dummyDelay);

  vld.write(false);

  // wait for target rdy goes to false
  while (rdy.read() == true) {
    wait(CLOCK_PERIOD, SC_NS);
  }
  // wait(sc_core::SC_ZERO_TIME);
  // wait(dummyDelay);
  // Wait for Simulation Delay of Target
  wait(2 * CLOCK_PERIOD, SC_NS);
} // do_trans()
