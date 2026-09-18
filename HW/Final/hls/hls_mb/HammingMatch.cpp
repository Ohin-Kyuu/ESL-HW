#include "memlib.h"
#include "HammingMatch.h"

inline sc_uint<6> popcnt32(sc_uint<32> x) {
  // Layer 0: 16, 2-bit Add
  sc_uint<2> L0[16];
  for (int i = 0; i < 16; i++)
    L0[i] = (sc_uint<2>)x[2 * i] + (sc_uint<2>)x[2 * i + 1];

  // Layer 1: 8, 3-bit Add
  sc_uint<3> L1[8];
  for (int i = 0; i < 8; i++)
    L1[i] = (sc_uint<3>)L0[2 * i] + (sc_uint<3>)L0[2 * i + 1];

  // Layer 2: 4, 4-bit Add
  sc_uint<4> L2[4];
  for (int i = 0; i < 4; i++)
    L2[i] = (sc_uint<4>)L1[2 * i] + (sc_uint<4>)L1[2 * i + 1];

  // Layer 3: 2, 2-bit Add
  sc_uint<5> L3[2];
  L3[0] = (sc_uint<5>)L2[0] + (sc_uint<5>)L2[1];
  L3[1] = (sc_uint<5>)L2[2] + (sc_uint<5>)L2[3];

  // Layer 4: 1, 6-bit Add (max = 32)
  return (sc_uint<6>)L3[0] + (sc_uint<6>)L3[1];
}

HammingMatch::HammingMatch(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_match);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);
#ifndef NATIVE_SYSTEMC
  i_train.clk_rst(i_clk, i_rst);
  i_query.clk_rst(i_clk, i_rst);
  o_best_idx.clk_rst(i_clk, i_rst);
  o_min_dist.clk_rst(i_clk, i_rst);
#endif
}

void HammingMatch::do_match() {
  // // Reset
  HLS_FLATTEN(qword);
  HLS_MAP_TO_MEMORY(train_db, "RAM_8X8_1RW");
  HLS_SPLIT_ARRAY_ADDR_LSB(train_db, 3);
  
  HLS_RESET_BLOCK(HLS_RESET_IN(i_train); HLS_RESET_IN(i_query);
                  HLS_RESET_OUT(o_best_idx); HLS_RESET_OUT(o_min_dist);)

  // Load Train DB
  for (int d = 0; d < 256; d++) {
    for (int w = 0; w < 8; w++) {
      HLS_PIPELINE("train_db_load");
      HLS_LAT_LOG("train_db_lat");
      HLS_READ(i_train, train_db[d][w]);
    }
  }

  // Process Query
  for (int q = 0; q < 256; q++) {
    HLS_DPO("do_match");

    for (int w = 0; w < 8; w++) {
      HLS_PIPELINE("qword_read");
      HLS_LAT_LOG("qword_lat");
      HLS_READ(i_query, qword[w]);
    }

    sc_uint<9> min_dist = 256;
    sc_uint<8> best_idx = 0;

    // For each Query, Match 256 Train
    for (int d = 0; d < 256; d++) {
      sc_uint<9> dist = 0;

      for (int w = 0; w < 8; w++) {
        HLS_PIPELINE("xor_loop");
        HLS_LAT_LOG("xor_lat");
        // HLS_UNROLL("xor_loop");

        sc_uint<32> xored = qword[w] ^ train_db[d][w];
        dist += popcnt32(xored);
      }

      // Update Min Hamming Distance
      if (dist < min_dist) {
        min_dist = dist;
        best_idx = (sc_uint<8>)d;
      }
    }

    HLS_WRITE(o_best_idx, best_idx);
    HLS_WRITE(o_min_dist, min_dist);
  }
}
