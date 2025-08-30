// Benchmark: frame sync quality metrics under noise
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lora_frame_sync.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void synth(uint32_t *s, size_t n, uint16_t pre_len, uint8_t sfd_len,
                  double pre_noise_pct, unsigned seed)
{
  // Background
  for (size_t i = 0; i < n; ++i) s[i] = (uint32_t)((i*5u + 9u) & 15u);
  // Preamble zeros
  for (uint16_t i = 0; i < pre_len && i < n; ++i) s[i] = 0;
  // SFD pattern
  for (uint8_t i = 0; i < sfd_len && pre_len + i < n; ++i) s[pre_len + i] = (uint32_t)(5 + (i & 1));
  // Inject noise into preamble run: flip some zeros to 1 based on pct
  if (pre_noise_pct > 0.0) {
    unsigned r = seed;
    for (uint16_t i = 0; i < pre_len; ++i) {
      r = r * 1103515245u + 12345u;
      double u = (double)(r >> 8) / (double)(1u << 24);
      if (u < pre_noise_pct) s[i] = 1; // non-zero-like glitch
    }
  }
}

int main(void) {
  const size_t nsym = 4096;
  const uint16_t pre_len = 16;
  const uint8_t sfd_len = 2;
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!syms) return 2;

  lora_fs_metrics m = {0};
  size_t idx;

  // Measure quality at three noise levels: 0%, 5%, 15%
  double noise[] = {0.0, 0.05, 0.15};
  const int iters = 20000;
  unsigned base_seed = 1u;

  puts("metric,value");
  for (int level = 0; level < 3; ++level) {
    double nlev = noise[level];
    unsigned seed = base_seed + (unsigned)level * 777u;
    double sum_pct = 0.0, sum_nz = 0.0;
    struct timespec t0, t1; double sec;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int r = 0; r < iters; ++r) {
      synth(syms, nsym, pre_len, sfd_len, nlev, seed + (unsigned)r);
      idx = lora_frame_sync_analyze(syms, nsym, pre_len, sfd_len, 4, &m);
      sum_pct += (double)m.preamble_match_pct;
      sum_nz  += (double)m.sfd_nonzero;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    sec = elapsed(t0, t1);
    double Mops = (double)iters / 1e6 / sec;
    double avg_pct = sum_pct / (double)iters;
    double avg_nz = sum_nz / (double)iters;
    printf("fsq_Mops_noise_%.0fpc,%.2f\n", nlev*100.0, Mops);
    printf("fsq_avg_preamble_match_pct_noise_%.0fpc,%.1f\n", nlev*100.0, avg_pct);
    printf("fsq_avg_sfd_nonzero_noise_%.0fpc,%.1f\n", nlev*100.0, avg_nz);
  }
  free(syms);
  return 0;
}

