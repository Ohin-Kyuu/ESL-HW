#ifndef HM_DEF_H_
#define HM_DEF_H_

#define CLOCK_PERIOD 10

// HammingMatch PE address map (offset from PE base)
// WRITE 0x00000000 - 0x00001FFF : train descriptors (256 × 32 B = 8192 B)
// WRITE 0x00002000 - 0x000027FF : query descriptors ( 64 × 32 B = 2048 B)
// READ  0x00004000 - 0x000041FF : results           ( 64 × 8  B =  512 B)

const int HM_TRAIN_OFFSET = 0x00000000;  // write: 8192 bytes
const int HM_QUERY_OFFSET = 0x00002000;  // write: 2048 bytes
const int HM_RESULT_OFFSET = 0x00004000; // read:   512 bytes

const int HM_TRAIN_SIZE = 8192; // 256 × 32 B
const int HM_QUERY_SIZE = 2048; //  64 × 32 B
const int HM_RESULT_SIZE = 512; //  64 × [best_idx:u32, min_dist:u32]

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

#endif // HM_DEF_H_
