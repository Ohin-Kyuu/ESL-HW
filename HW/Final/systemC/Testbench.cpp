#include "Testbench.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

Testbench::Testbench(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_tb);
  sensitive << i_clk.pos();
  dont_initialize();
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

void Testbench::verify() {
  printf("\n=== SW Golden Reference Verification ===\n");

  int errors = 0;
  uint32_t total_hw_dist = 0;

  for (int q = 0; q < 256; q++) {
    // Compute exact match in software
    uint32_t sw_min = 256;
    int sw_idx = 0;

    for (int d = 0; d < 256; d++) {
      uint32_t dist = 0;
      for (int w = 0; w < 8; w++) {
        uint32_t x = static_cast<uint32_t>(query_db[q][w]) ^
                     static_cast<uint32_t>(train_db[d][w]);
        dist += static_cast<uint32_t>(__builtin_popcount(x));
      }
      if (dist < sw_min) {
        sw_min = dist;
        sw_idx = d;
      }
    }

    int hw_idx = static_cast<int>(best_idx_buf[q]);
    int hw_dist = static_cast<int>(min_dist_buf[q]);
    total_hw_dist += hw_dist;

    if (hw_dist != static_cast<int>(sw_min) || hw_idx != sw_idx) {
      printf("  MISMATCH q=%3d: HW(idx=%3d, dist=%3d) "
             "!= SW(idx=%3d, dist=%3d)\n",
             q, hw_idx, hw_dist, sw_idx, static_cast<int>(sw_min));
      errors++;
    }
  }

  printf("Average min-distance: %.1f\n",
         static_cast<double>(total_hw_dist) / 256.0);

  if (errors == 0)
    printf("PASS: All 256 queries match SW golden reference\n");
  else
    printf("FAIL: %d / 256 queries mismatched\n", errors);
}

void Testbench::do_tb() {
  // Reset
  o_rst.write(false);
  wait();
  o_rst.write(true);

  // // Load Train
  // for (int d = 0; d < 256; d++)
  //   for (int w = 0; w < 8; w++)
  //     o_train.write(train_db[d][w]);

  // Process Query
  for (int q = 0; q < 256; q++) {

    for (int w = 0; w < 8; w++)
      o_query.write(query_db[q][w]);

    // Load Train
    for (int d = 0; d < 256; d++)
      for (int w = 0; w < 8; w++)
        o_train.write(train_db[d][w]);

    best_idx_buf[q] = i_best_idx.read();
    min_dist_buf[q] = i_min_dist.read();
  }

  sc_stop();
}
