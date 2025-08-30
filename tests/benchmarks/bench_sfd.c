// Micro-benchmark: SFD detection (adaptive lookahead)
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lora_frame_sync.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void make_seq(uint32_t *s, size_t n, size_t pre, size_t sfd_len, unsigned noise)
{
  for (size_t i = 0; i < n; ++i) s[i] = (uint32_t)(((i*11u)+3u) & 15u);
  for (size_t i = 0; i < pre && i < n; ++i) s[i] = 0;
  for (size_t i = 0; i < sfd_len && pre + i < n; ++i) s[pre + i] = (uint32_t)(5 + (i & 1));
  // inject up to `noise` preamble-like zeros randomly inside SFD region tail
  for (unsigned k = 0; k < noise; ++k) {
    size_t idx = pre + (k % (sfd_len ? sfd_len : 1));
    if (idx < n) s[idx] = 0;
  }
}

int main(void) {
  const size_t nsym = 2048;
  const uint16_t pre_len = 16;
  const uint8_t sfd_len = 2;
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!syms) return 2;
  make_seq(syms, nsym, pre_len, sfd_len, 0);

  // Sanity
  size_t pe = lora_frame_sync_find_preamble(syms, nsym, pre_len);
  size_t se = lora_frame_sync_find_sfd(syms, nsym, pe, sfd_len, 4);
  if (se != pre_len + sfd_len) {
    fprintf(stderr, "sanity failed: se=%zu expected=%u\n", se, (unsigned)(pre_len + sfd_len));
    return 3;
  }

  const int iters = 300000;
  struct timespec t0, t1; double sec;

  // Inject a bit of noise inside SFD for robustness benchmark
  make_seq(syms, nsym, pre_len, sfd_len, 1);
  // Measure with lookahead=2
  clock_gettime(CLOCK_MONOTONIC, &t0);
  size_t acc = 0;
  for (int r = 0; r < iters; ++r) {
    size_t pe2 = lora_frame_sync_find_preamble(syms, nsym, pre_len);
    acc += lora_frame_sync_find_sfd(syms, nsym, pe2, sfd_len, 2);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  double Mops2 = (double)iters / 1e6 / sec;

  // Measure with lookahead=8
  clock_gettime(CLOCK_MONOTONIC, &t0);
  size_t acc2 = 0;
  for (int r = 0; r < iters; ++r) {
    size_t pe2 = lora_frame_sync_find_preamble(syms, nsym, pre_len);
    acc2 += lora_frame_sync_find_sfd(syms, nsym, pe2, sfd_len, 8);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  sec = elapsed(t0, t1);
  double Mops8 = (double)iters / 1e6 / sec;

  puts("metric,value");
  printf("sfd_Mops_look2,%.2f\n", Mops2);
  printf("sfd_Mops_look8,%.2f\n", Mops8);
  printf("sfd_idx_sum_look2,%zu\n", acc);
  printf("sfd_idx_sum_look8,%zu\n", acc2);
  free(syms);
  return 0;
}
