#include "Target.h"
#include "filter_def.h"

Target::Target(sc_module_name n)
    : sc_module(n), tsock("tsock"), base_offset(0) {
  tsock.register_b_transport(this, &Target::blocking_transport);
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

void Target::blocking_transport(tlm::tlm_generic_payload &payload,
                                sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();

  // Burst need to know length
  unsigned int data_len = payload.get_data_length();

  switch (payload.get_command()) {
  // READ
  case tlm::TLM_READ_COMMAND: {
    if (addr >= MOD_RESULT_ADDR) {
      for (unsigned int i = 0; i < data_len; ++i) {
        // data_ptr[i] = result_buf[i];
        if (!result_buf.empty()) {
          data_ptr[i] = result_buf.front();
          result_buf.pop();
        } else {
          data_ptr[i] = 0;
        }
      }
    } else {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
    // Delay
    unsigned int rc = (data_len == 1) ? 1 : (2 + data_len);
    readout_cycles += rc;
    delay += sc_time(rc * CLOCK_PERIOD, SC_NS);
    break;
  }

  // WRITE
  case tlm::TLM_WRITE_COMMAND:
    if (addr == MOD_WIDTH_ADDR) {
      // Reset
      this->width = *((unsigned int *)data_ptr);
      med_buf.assign(3, std::vector<unsigned char>(this->width, 0));
      sch_buf.assign(3, std::vector<unsigned char>(this->width, 0));
      this->x = 0;
      this->y = 0;
      std::queue<unsigned char> empty;
      std::swap(result_buf, empty);
      delay += sc_time(CLOCK_PERIOD, SC_NS);

    } else if (addr == MOD_HEIGHT_ADDR) {
      this->height = *((unsigned int *)data_ptr);
      delay += sc_time(CLOCK_PERIOD, SC_NS);
    } else if (addr >= MOD_INPUT_ADDR && addr <= MOD_RESULT_ADDR) {
      for (unsigned int i = 0; i < data_len; ++i) {
        if (mask_ptr == nullptr || mask_ptr[i] == 0xff) {
          // result_buf[i] = 0;

          med_buf[y % 3][x] = data_ptr[i];

          if (y >= 2 && x >= 2) {
            int med_x = x - 2;
            int med_y = y - 2;

            unsigned char med_val = do_median(med_x, med_y);
            sch_buf[med_y % 3][med_x] = med_val;

            if (med_y >= 2 && med_x >= 2) {
              int sch_x = med_x - 2;
              int sch_y = med_y - 2;

              // result_buf[i] = (unsigned char)do_scharr(sch_x, sch_y);
              result_buf.push((unsigned char)do_scharr(sch_x, sch_y));
            }
          }

          x++;
          if (x >= width) {
            x = 0;
            y++;
          }
        }
      } // for loop end
      // HLS timing results
      double total_hw_cycles = 2.07 * data_len;
      filter_cycles += (uint64_t)total_hw_cycles; // sum
      delay += sc_time(total_hw_cycles * CLOCK_PERIOD, SC_NS);

    } else {
      payload.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
    break;

  // IGNORE
  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }

  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
