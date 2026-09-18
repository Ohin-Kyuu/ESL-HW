#include "assert.h"
#include "irq.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "query_desc.h"
#include "train_desc.h"

inline unsigned int read_cycles() {
  unsigned int cycles;
  asm volatile("csrr %0, mcycle" : "=r"(cycles));
  return cycles;
}

// Timer
static void set_next_timer_interrupt() {
  assert(mtime && mtimecmp);
  *mtimecmp = *mtime + 1000;
}
unsigned int num_ticks = 0;
void timer_irq_handler() {
  set_next_timer_interrupt();
  ++num_ticks;
}

// PE interrupt handlers
bool acc_completed[4] = {false};
void acc_irq_handler_0() { acc_completed[0] = true; }
void acc_irq_handler_1() { acc_completed[1] = true; }
void acc_irq_handler_2() { acc_completed[2] = true; }
void acc_irq_handler_3() { acc_completed[3] = true; }

void init_acc_irq_handler() {
  register_interrupt_handler(6, acc_irq_handler_0);
  register_interrupt_handler(8, acc_irq_handler_1);
  register_interrupt_handler(9, acc_irq_handler_2);
  register_interrupt_handler(10, acc_irq_handler_3);
}

// Semaphores
int sem_init(uint32_t *s, uint32_t v) {
  *s = v;
  return 0;
}

int sem_wait(uint32_t *__sem) {
  uint32_t value, success;
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w  %[value],(%[__sem])\n\t\
     beqz  %[value],L%=\n\t\
     addi  %[value],%[value],-1\n\t\
     sc.w  %[success],%[value],(%[__sem])\n\t\
     bnez  %[success],L%=\n\t"
                       : [value] "=r"(value), [success] "=r"(success)
                       : [__sem] "r"(__sem)
                       : "memory");
  return 0;
}

int sem_post(uint32_t *__sem) {
  uint32_t value, success;
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w  %[value],(%[__sem])\n\t\
     addi  %[value],%[value],1\n\t\
     sc.w  %[success],%[value],(%[__sem])\n\t\
     bnez  %[success],L%=\n\t"
                       : [value] "=r"(value), [success] "=r"(success)
                       : [__sem] "r"(__sem)
                       : "memory");
  return 0;
}

int barrier(uint32_t *sem, uint32_t *lk, uint32_t *cnt, uint32_t n) {
  sem_wait(lk);
  if (*cnt == n - 1) {
    *cnt = 0;
    sem_post(lk);
    for (int j = 0; j < (int)(n - 1); j++)
      sem_post(sem);
  } else {
    (*cnt)++;
    sem_post(lk);
    sem_wait(sem);
  }
  return 0;
}

// Shared sync objects
#define PROCESSORS 4
uint32_t barrier_counter = 0;
uint32_t barrier_lock, barrier_sem;
uint32_t dma_lock;
uint32_t print_lock;
uint32_t print_sem[PROCESSORS];

// PE addresses: HammingMatchPE
// memory map (offset from base):
//   0x00000000 : WRITE train (8192 B)
//   0x00002000 : WRITE query (2048 B) → do_compute
//   0x00004000 : READ  results (512 B)
static char *const PE_BASE[4] = {
    (char *)0x73000000,
    (char *)0x74000000,
    (char *)0x75000000,
    (char *)0x76000000,
};
static char *const PE_RESULT_ADDR[4] = {
    (char *)(0x73000000 + 0x4000),
    (char *)(0x74000000 + 0x4000),
    (char *)(0x75000000 + 0x4000),
    (char *)(0x76000000 + 0x4000),
};

// DMA
static volatile uint32_t *const DMA_SRC_ADDR = (uint32_t *)0x70000000;
static volatile uint32_t *const DMA_DST_ADDR = (uint32_t *)0x70000004;
static volatile uint32_t *const DMA_LEN_ADDR = (uint32_t *)0x70000008;
static volatile uint32_t *const DMA_OP_ADDR = (uint32_t *)0x7000000C;
static const uint32_t DMA_OP_MEMCPY = 1;
bool _is_using_dma = true;

void write_data_to_ACC(char *ADDR, unsigned char *buf, int len) {
  if (_is_using_dma) {
    *DMA_SRC_ADDR = (uint32_t)buf;
    *DMA_DST_ADDR = (uint32_t)ADDR;
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR = DMA_OP_MEMCPY;
  } else {
    memcpy(ADDR, buf, len);
  }
}
void read_data_from_ACC(char *ADDR, unsigned char *buf, int len) {
  if (_is_using_dma) {
    *DMA_SRC_ADDR = (uint32_t)ADDR;
    *DMA_DST_ADDR = (uint32_t)buf;
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR = DMA_OP_MEMCPY;
  } else {
    memcpy(buf, ADDR, len);
  }
}

union word {
  unsigned int uint;
  unsigned char uc[4];
};

// Send buffer: train(8192) + query_slice(2048)
// Indexed by hart_id to avoid inter-hart data race.
// Size: 4 × 10240 = 40 KB in BSS segment.
static unsigned char send_buf[4][10240];

int total_error = 0;

void prepare_data(unsigned hart_id, int q_start) {
  memcpy(send_buf[hart_id], (void *)train_descriptors, 8192);
  memcpy(send_buf[hart_id] + 8192, (void *)&query_descriptors[q_start][0],
         2048);
}

void sw_compute(int q_start, uint32_t *sw_best_idx) {
  for (int i = 0; i < 64; i++) {
    uint32_t ref_best = 0, ref_min = 256;
    for (int d = 0; d < 256; d++) {
      uint32_t dist = 0;
      for (int w = 0; w < 8; w++) {
        uint32_t qw = 0, tw = 0;
        for (int b = 0; b < 4; b++) {
          qw |= ((uint32_t)query_descriptors[q_start + i][w * 4 + b])
                << (b * 8);
          tw |= ((uint32_t)train_descriptors[d][w * 4 + b]) << (b * 8);
        }
        dist += (uint32_t)__builtin_popcount(qw ^ tw);
      }
      if (dist < ref_min) {
        ref_min = dist;
        ref_best = d;
      }
    }
    sw_best_idx[i] = ref_best;
  }
}

int verify(unsigned char *result_buf, uint32_t *sw_best_idx,
           uint32_t *hw_best_idx, uint32_t *hw_min_dist) {
  int local_error = 0;
  word best_idx_w, min_dist_w;

  for (int i = 0; i < 64; i++) {
    best_idx_w.uc[0] = result_buf[i * 8 + 0];
    best_idx_w.uc[1] = result_buf[i * 8 + 1];
    best_idx_w.uc[2] = result_buf[i * 8 + 2];
    best_idx_w.uc[3] = result_buf[i * 8 + 3];

    min_dist_w.uc[0] = result_buf[i * 8 + 4];
    min_dist_w.uc[1] = result_buf[i * 8 + 5];
    min_dist_w.uc[2] = result_buf[i * 8 + 6];
    min_dist_w.uc[3] = result_buf[i * 8 + 7];

    hw_best_idx[i] = best_idx_w.uint;
    hw_min_dist[i] = min_dist_w.uint;

    if (hw_best_idx[i] != sw_best_idx[i]) {
      local_error++;
    }
  }
  return local_error;
}

void write_csv(unsigned hart_id, int q_start, int local_error,
               uint32_t *hw_best_idx, uint32_t *hw_min_dist) {

  if (hart_id != 0)
    sem_wait(&print_sem[hart_id]);
  else {
    printf("num_ticks = %d\n", num_ticks);
    printf("total_error = %d\n", total_error);
  }

  FILE *fp =
      (hart_id == 0) ? fopen("results.csv", "w") : fopen("results.csv", "a");
  if (fp) {
    if (hart_id == 0)
      fprintf(fp, "query_idx,best_train_idx,min_hamming_dist\n");
    for (int i = 0; i < 64; i++) {
      fprintf(fp, "%d,%u,%u\n", q_start + i, hw_best_idx[i], hw_min_dist[i]);
    }
    fclose(fp);
  } else {
    printf("core%u: Failed to open results.csv for writing!\n", hart_id);
  }
}

int main(unsigned hart_id) {

  if (hart_id == 0) {
    register_timer_interrupt_handler(timer_irq_handler);
    set_next_timer_interrupt();
    init_acc_irq_handler();

    sem_init(&barrier_lock, 1);
    sem_init(&barrier_sem, 0);
    sem_init(&dma_lock, 1);
    sem_init(&print_lock, 1);
    for (int i = 0; i < PROCESSORS; i++)
      sem_init(&print_sem[i], 0);
  }

  int q_start = hart_id * 64;

  unsigned int t_start = read_cycles();
  // Prepare Data
  prepare_data(hart_id, q_start);

  // DMA Write
  unsigned int t0 = read_cycles();
  sem_wait(&dma_lock);
  write_data_to_ACC(PE_BASE[hart_id], send_buf[hart_id], 10240);
  sem_post(&dma_lock);
  unsigned int t1 = read_cycles();

  // PE compute
  while (!acc_completed[hart_id])
    ;
  acc_completed[hart_id] = false;
  unsigned int t2 = read_cycles();

  // DMA Read
  unsigned char result_buf[512];
  sem_wait(&dma_lock);
  read_data_from_ACC(PE_RESULT_ADDR[hart_id], result_buf, 512);
  sem_post(&dma_lock);
  unsigned int t3 = read_cycles();

  // SW compute
  uint32_t sw_best_idx[64];
  sw_compute(q_start, sw_best_idx);
  unsigned int t4 = read_cycles();

  // Verify Results
  uint32_t hw_best_idx[64];
  uint32_t hw_min_dist[64];
  int local_error = verify(result_buf, sw_best_idx, hw_best_idx, hw_min_dist);

  // Sync
  sem_wait(&print_lock);
  total_error += local_error;
  sem_post(&print_lock);
  barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);

  // Write CSV
  write_csv(hart_id, q_start, local_error, hw_best_idx, hw_min_dist);

  // Print
  unsigned int prepare_cycles = t0 - t_start;
  unsigned int dma_write_cycles = t1 - t0;
  unsigned int hw_compute_cycles = t2 - t1;
  unsigned int dma_read_cycles = t3 - t2;
  unsigned int sw_compute_cycles = t4 - t3;
  unsigned int total_cycles = t3 - t_start;
  double speedup = (double)sw_compute_cycles / (double)hw_compute_cycles;
  double t_speedup = (double)sw_compute_cycles / (double)total_cycles;

  printf("core%u: q[%d..%d] done, local_error=%d\n", hart_id, q_start,
         q_start + 63, local_error);
  printf("  [Core %u Profiling]\n", hart_id);
  printf("  ============================\n");
  printf("  - Prepare Data: %u cycles\n", prepare_cycles);
  printf("  - DMA Write   : %u cycles\n", dma_write_cycles);
  printf("  - HW Compute  : %u cycles\n", hw_compute_cycles);
  printf("  - DMA Read    : %u cycles\n", dma_read_cycles);
  printf("  - TOTAL       : %u cycles\n", total_cycles);
  printf("  ----------------------------\n");
  printf("  - SW Compute     : %u cycles\n", sw_compute_cycles);
  printf("  - Compute Speedup: %.2f x\n", speedup);
  printf("  - True Speedup   : %.2f x\n", t_speedup);
  printf("  ============================\n\n");
  // Next Core
  if (hart_id + 1 < PROCESSORS)
    sem_post(&print_sem[hart_id + 1]);

  return 0;
}
