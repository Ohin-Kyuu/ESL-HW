#include "Target.h"
#include "filter_def.h"

using namespace std;

// scharr mask
const int mask[MASK_N][MASK_Y][MASK_X] = {
    {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}}, // G_x
    {{-3, -10, -3}, {0, 0, 0}, {3, 10, 3}}  // G_y
};

inline unsigned char med3(unsigned char a, unsigned char b, unsigned char c) {
  if ((a <= b && b <= c) || (c <= b && b <= a))
    return b;
  if ((b <= a && a <= c) || (c <= a && a <= b))
    return a;
  return c;
}

Target::Target(sc_module_name n, unsigned int w, unsigned int h)
    : sc_module(n), t_skt("t_skt"), base_offset(0), width(w + 4), height(h + 4),
      m_peq("peq") {

  med_buf.resize(3, std::vector<unsigned char>(width)); // [3 row, width]
  sch_buf.resize(3, std::vector<unsigned char>(width)); // [3 row, width]

  // Socket
  t_skt.register_nb_transport_fw(this, &Target::nb_transport_fw);
  // Register PEQ thread
  SC_THREAD(peq_process_thread);
}

tlm::tlm_sync_enum Target::nb_transport_fw(tlm::tlm_generic_payload &payload,
                                           tlm::tlm_phase &phase,
                                           sc_core::sc_time &delay) {
  m_peq.notify(payload, delay);
  return tlm::TLM_ACCEPTED;
}

unsigned char Target::do_median(int u, int v) {
  unsigned char row_med[3];
  for (int i = 0; i < MASK_Y; ++i) {
    unsigned char a = med_buf[(v + i) % 3][u];
    unsigned char b = med_buf[(v + i) % 3][u + 1];
    unsigned char c = med_buf[(v + i) % 3][u + 2];
    row_med[i] = med3(a, b, c);
  }

  return med3(row_med[0], row_med[1], row_med[2]);
}

int Target::do_scharr(int u, int v) {
  int val[MASK_N] = {0};

  for (int i = 0; i < MASK_Y; ++i) {
    for (int j = 0; j < MASK_X; ++j) {
      unsigned char grey = sch_buf[(v + i) % 3][u + j];

      for (int k = 0; k < MASK_N; ++k) {
        val[k] += grey * mask[k][i][j];
      }
    }
  }

  double total = 0.0;
  for (int i = 0; i < MASK_N; ++i) {
    total += (double)val[i] * (double)val[i];
  }

  double grad = std::sqrt(total);
  int result = (int)(std::round(grad));

  // clip
  if (result > 255)
    result = 255;
  if (result < 0)
    result = 0;

  return result;
}

void Target::peq_process_thread() {
  while (true) {
    // Wait Until Notify
    wait(m_peq.get_event());
    tlm::tlm_generic_payload *payload;

    while ((payload = m_peq.get_next_transaction()) != nullptr) {
      // cout << "[Target] PEQ Triggered! Start processing at " <<
      // sc_time_stamp()
      //      << endl;
      sc_dt::uint64 addr = payload->get_address();
      addr = addr - base_offset;
      unsigned char *mask_ptr = payload->get_byte_enable_ptr();
      unsigned char *data_ptr = payload->get_data_ptr();

      switch (payload->get_command()) {
      // READ
      case tlm::TLM_READ_COMMAND:
        if (addr == MOD_RESULT_ADDR) {
          data_ptr[0] = (unsigned char)this->result;
        } else {
          payload->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }
        wait(payload->get_data_length() * 2 * CLOCK_PERIOD, sc_core::SC_NS);
        break;

      // WRITE
      case tlm::TLM_WRITE_COMMAND:
        if (addr == MOD_INPUT_ADDR) {
          if (mask_ptr[0] == 0xff) {
            med_buf[y % 3][x] = data_ptr[0];

            if (y >= 2 && x >= 2) {
              int med_x = x - 2;
              int med_y = y - 2;

              unsigned char med_val = do_median(med_x, med_y);
              sch_buf[med_y % 3][med_x] = med_val;

              if (med_y >= 2 && med_x >= 2) {
                int sch_x = med_x - 2;
                int sch_y = med_y - 2;

                this->result = do_scharr(sch_x, sch_y);
              }
            }

            x++;
            if (x >= width) {
              x = 0;
              y++;
            }
          }
        } else {
          payload->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        }
        wait(payload->get_data_length() * 2 * CLOCK_PERIOD, sc_core::SC_NS);
        break;

      // IGNORE
      case tlm::TLM_IGNORE_COMMAND:
        payload->set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        continue;
      default:
        payload->set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        continue;
      }

      payload->set_response_status(tlm::TLM_OK_RESPONSE); // Always OK

      // cout << "[Target] Work Done. Send BW_Transport at " << sc_time_stamp()
      // << endl;

      // Return RESP
      tlm::tlm_phase bw_phase = tlm::BEGIN_RESP;
      sc_core::sc_time bw_delay = sc_core::SC_ZERO_TIME;

      t_skt->nb_transport_bw(*payload, bw_phase, bw_delay);
    }
  }
}
