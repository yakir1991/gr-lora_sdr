// Micro-benchmark: frame sync presets per SF
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "lora_frame_sync.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void make_seq(uint32_t *s, size_t n, uint16_t pre, uint8_t sfd_len) {
  for (size_t i = 0; i < n; ++i) s[i] = (uint32_t)((i*13u+7u) & 15u);
  for (uint16_t i = 0; i < pre && i < n; ++i) s[i] = 0;
  for (uint8_t i = 0; i < sfd_len && pre + i < n; ++i) s[pre + i] = (uint32_t)(5 + (i & 1));
}

int main(void) {
  const size_t nsym = 2048;
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!syms) return 2;
  puts("metric,value");
  for (uint8_t sf = 7; sf <= 12; ++sf) {
    uint16_t pre = 8u;
    uint8_t sfd_len = 2u;
    make_seq(syms, nsym, pre, sfd_len);

    lora_fs_cfg cfg; lora_frame_sync_recommend(sf, 125000, &cfg);
    struct timespec t0, t1; double sec;
    const int iters = 200000;
    // Bench CFG variant
    clock_gettime(CLOCK_MONOTONIC, &t0);
    size_t acc = 0;
    for (int r = 0; r < iters; ++r) {
      size_t pe = lora_frame_sync_find_preamble_cfg(syms, nsym, pre, &cfg);
      acc += lora_frame_sync_find_sfd_cfg(syms, nsym, pe, &cfg);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    sec = elapsed(t0, t1);
    double Mcfg = (double)iters / 1e6 / sec;
    printf("fs_presets_sf%u_Mops,%.2f\n", sf, Mcfg);
  }
  free(syms);
  return 0;
}

