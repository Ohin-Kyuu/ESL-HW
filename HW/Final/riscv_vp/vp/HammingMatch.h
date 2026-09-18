#ifndef HAMMING_MATCH_H_
#define HAMMING_MATCH_H_

#include <cstring>
#include <systemc>
using namespace sc_core;

#include "core/common/irq_if.h"
#include "hm_def.h"
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

struct HammingMatch : public sc_module {
  tlm_utils::simple_target_socket<HammingMatch> tsock;

  interrupt_gateway *plic = 0;
  uint32_t irq_number = 0;

  uint32_t train_db[256][8]; // Memory bank
  uint32_t query_buf[64][8]; // 64 queries
  uint32_t results[64 * 2];  // [best_idx, min_dist] × 64

  unsigned int total_compute_cycles = 0;
  sc_fifo<bool> compute_start_fifo;

  SC_HAS_PROCESS(HammingMatch);

  HammingMatch(sc_module_name n, uint32_t irq_number)
      : sc_module(n), irq_number(irq_number), compute_start_fifo(1) {
    tsock.register_b_transport(this, &HammingMatch::blocking_transport);
    SC_THREAD(do_compute);
  }

  void blocking_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay) {
    wait(delay);
    auto cmd = payload.get_command();
    auto addr = payload.get_address();
    auto len = payload.get_data_length();
    auto *ptr = payload.get_data_ptr();

    switch (cmd) {
    case tlm::TLM_WRITE_COMMAND:
      if (addr < HM_QUERY_OFFSET) {
        memcpy((uint8_t *)train_db + addr, ptr, len);
      } else if (addr < HM_RESULT_OFFSET) {
        memcpy((uint8_t *)query_buf + (addr - HM_QUERY_OFFSET), ptr, len);
        if (addr + len >= HM_QUERY_OFFSET + HM_QUERY_SIZE)
          compute_start_fifo.nb_write(true);
      }
      break;
    case tlm::TLM_READ_COMMAND:
      if (addr >= HM_RESULT_OFFSET)
        memcpy(ptr, (uint8_t *)results + (addr - HM_RESULT_OFFSET), len);
      break;
    default:
      payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
    payload.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  void do_compute() {
    while (true) {
      compute_start_fifo.read();

      for (int q = 0; q < 64; q++) {
        uint32_t min_dist = 256, best_idx = 0;
        for (int d = 0; d < 256; d++) {
          uint32_t dist = 0;
          for (int w = 0; w < 8; w++)
            dist +=
                (uint32_t)__builtin_popcount(query_buf[q][w] ^ train_db[d][w]);
          if (dist < min_dist) {
            min_dist = dist;
            best_idx = d;
          }
        }
        results[q * 2 + 0] = best_idx;
        results[q * 2 + 1] = min_dist;
      }
      wait(183040, SC_NS); // DPO config
      total_compute_cycles += 18304;
      plic->gateway_trigger_interrupt(irq_number);
    }
  }
};

#endif // HAMMING_MATCH_H_
