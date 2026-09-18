#ifndef SIMPLEBUS_H_
#define SIMPLEBUS_H_

#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

#include "MemoryMap.h"

template <int NR_OF_TARGET_SOCKETS, int NR_OF_INITIATOR_SOCKETS>
class SimpleBus : public sc_core::sc_module, public MemoryMap {
public:
  // Socket
  using target_socket_type = tlm_utils::simple_target_socket_tagged<SimpleBus>;
  using initiator_socket_type =
      tlm_utils::simple_initiator_socket_tagged<SimpleBus>;

  target_socket_type t_skt[NR_OF_TARGET_SOCKETS];
  initiator_socket_type i_skt[NR_OF_INITIATOR_SOCKETS];

  unsigned int r_cnt = 0;
  unsigned int w_cnt = 0;

  SC_CTOR(SimpleBus) : MemoryMap(this->name(), NR_OF_INITIATOR_SOCKETS) {
    for (unsigned int i = 0; i < NR_OF_TARGET_SOCKETS; ++i) {
      t_skt[i].register_b_transport(this, &SimpleBus::initiatorBTransport, i);
    }
  }

  void initiatorBTransport(int SocketId, tlm::tlm_generic_payload &trans,
                           sc_core::sc_time &t) {

    Addr orig = trans.get_address();
    Addr offset;
    int portId = getPortId(orig, offset);

    if (portId < 0 || portId >= NR_OF_INITIATOR_SOCKETS) {
      std::cout << "ERROR: " << name() << ": initiatorBTransport()"
                << ": Invalid (undefined memory mapped) address == 0x"
                << std::hex << trans.get_address() << std::dec << std::endl;
      assert(false);
    }

    if (trans.is_read()) {
      r_cnt++;
    } else if (trans.is_write()) {
      w_cnt++;
    }

    trans.set_address(offset);

    i_skt[portId]->b_transport(trans, t);
  }

  void report() {
    std::cout << name() << " Counts:" << std::endl;
    std::cout << "  Read:  " << r_cnt << std::endl;
    std::cout << "  Write: " << w_cnt << std::endl;
  }
};

#endif // !SIMPLEBUS_H_
