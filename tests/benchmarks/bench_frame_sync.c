// Micro-benchmark: Frame sync preamble finder
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lora_frame_sync.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void synth(uint32_t *syms, size_t n, size_t pre_off, size_t pre_len) {
  for (size_t i = 0; i < n; ++i) syms[i] = (uint32_t)((i*7u) & 15u);
  for (size_t i = 0; i < pre_len && pre_off + i < n; ++i) syms[pre_off + i] = 0;
}

int main(void) {
  const size_t nsym = 4096;
  const size_t pre_len = 64;
  const size_t pre_off = 123;
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!syms) return 2;
  synth(syms, nsym, pre_off, pre_len);

  // Correctness quick check
  size_t end = lora_frame_sync_find_preamble(syms, nsym, (uint16_t)pre_len);
  if (end != pre_off + pre_len) {
    fprintf(stderr, "mismatch: got %zu expected %zu\n", end, pre_off + pre_len);
    return 3;
  }

  const int iters = 200000;
  struct timespec t0, t1; double sec;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  size_t acc = 0;
  for (int r = 0; r < iters; ++r) acc += lora_frame_sync_find_preamble(syms, nsym, (uint16_t)pre_len);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  double Mops = (double)iters / 1e6 / sec;

  puts("metric,value");
  printf("frame_sync_find_preamble_Mops,%.2f\n", Mops);
  printf("frame_sync_idx_sum,%zu\n", acc);

  free(syms);
  return 0;
}

