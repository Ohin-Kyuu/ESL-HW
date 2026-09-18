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
}

void HammingMatch::do_match() {
  // // Reset
  // for (int d = 0; d < 256; d++)
  //   for (int w = 0; w < 8; w++)
  //     train_db[d][w] = 0;
  //
  // // Load Train: 256 point
  // for (int d = 0; d < 256; d++)
  //   for (int w = 0; w < 8; w++)
  //     train_db[d][w] = i_train.read();

  // Process Query
  for (int q = 0; q < 256; q++) {
    for (int w = 0; w < 8; w++)
      qword[w] = i_query.read();

    sc_uint<9> min_dist = 256;
    sc_uint<8> best_idx = 0;

    // For each Query, Match 256 Train
    for (int d = 0; d < 256; d++) {
      // No Memory Bank
      for (int w = 0; w < 8; w++)
        tword[w] = i_train.read();

      sc_uint<9> dist = 0;
      for (int w = 0; w < 8; w++) {
        // sc_uint<32> xored = qword[w] ^ train_db[d][w];
        sc_uint<32> xored = qword[w] ^ tword[w];
        dist += popcnt32(xored);
      }

      // Update Min Hamming Distance
      if (dist < min_dist) {
        min_dist = dist;
        best_idx = (sc_uint<8>)d;
      }
    }

    o_best_idx.write(best_idx);
    o_min_dist.write(min_dist);
    wait();
  }
}
