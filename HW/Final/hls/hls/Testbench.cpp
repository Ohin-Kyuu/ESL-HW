#include "Testbench.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sysc/kernel/sc_time.h>

using namespace std;

Testbench::Testbench(sc_module_name n)
    : sc_module(n), total_run_time(SC_ZERO_TIME),
      total_start_time(SC_ZERO_TIME) {
  SC_THREAD(feed);
  sensitive << i_clk.pos();
  dont_initialize();

  SC_THREAD(fetch);
  sensitive << i_clk.pos();
  dont_initialize();

#ifndef NATIVE_SYSTEMC
  o_train.clk_rst(i_clk, o_rst);
  o_query.clk_rst(i_clk, o_rst);
#endif
}

// Parse one txt file (256 lines × 32 bytes) into data[256][8]
// Each 4 consecutive bytes are packed little-endian into one sc_uint<32> word.
int Testbench::load_descriptors(const std::string &filename,
                                sc_uint<32> data[256][8]) {
  std::ifstream f(filename);
  if (!f.is_open()) {
    printf("[TB] Cannot open: %s\n", filename.c_str());
    return -1;
  }

  std::string line;
  int d = 0;
  while (std::getline(f, line) && d < 256) {
    uint8_t bytes[32] = {};
    std::istringstream ss(line);
    int val;
    char comma;
    for (int i = 0; i < 32; i++) {
      if (!(ss >> val))
        break;
      bytes[i] = static_cast<uint8_t>(val);
      if (i < 31)
        ss >> comma;
    }

    // Pack: bytes[w*4 .. w*4+3] → word[w] (little-endian)
    for (int w = 0; w < 8; w++) {
      data[d][w] = static_cast<sc_uint<32>>((uint32_t)bytes[w * 4 + 0] |
                                            ((uint32_t)bytes[w * 4 + 1] << 8) |
                                            ((uint32_t)bytes[w * 4 + 2] << 16) |
                                            ((uint32_t)bytes[w * 4 + 3] << 24));
    }
    d++;
  }

  printf("[TB] Loaded %d descriptors from %s\n", d, filename.c_str());
  return (d == 256) ? 0 : -1;
}

int Testbench::read_train(const std::string &filename) {
  return load_descriptors(filename, train_db);
}

int Testbench::read_query(const std::string &filename) {
  return load_descriptors(filename, query_db);
}

void Testbench::write_results(const std::string &filename) {
  FILE *fp = fopen(filename.c_str(), "w");
  if (!fp) {
    printf("[TB] Cannot write: %s\n", filename.c_str());
    return;
  }
  fprintf(fp, "query_idx,best_train_idx,min_hamming_dist\n");
  for (int q = 0; q < 256; q++)
    fprintf(fp, "%d,%d,%d\n", q, static_cast<int>(best_idx_buf[q]),
            static_cast<int>(min_dist_buf[q]));
  fclose(fp);
  printf("[TB] Results written to %s\n", filename.c_str());
}

void Testbench::feed() {
  // Reset
  HLS_RESET_OUT(o_train);
  HLS_RESET_OUT(o_query);

  o_rst.write(false);
  wait(5);
  o_rst.write(true);
  wait(1);

  total_start_time = sc_time_stamp();

  // Feed Query
  for (int q = 0; q < 256; q++) {

    for (int w = 0; w < 8; w++)
      HLS_WRITE(o_query, query_db[q][w]);

    // Send 256 Train
    for (int d = 0; d < 256; d++)
      for (int w = 0; w < 8; w++)
        HLS_WRITE(o_train, train_db[d][w]);
  }
}

void Testbench::fetch() {
  // Reset
  HLS_RESET_IN(i_best_idx);
  HLS_RESET_IN(i_min_dist);

  wait(5);
  wait(1);

  // Fetch Results
  for (int q = 0; q < 256; q++) {
    HLS_READ(i_best_idx, best_idx_buf[q]);
    HLS_READ(i_min_dist, min_dist_buf[q]);
  }

  total_run_time = sc_time_stamp() - total_start_time;
  sc_stop();
}
