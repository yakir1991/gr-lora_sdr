#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <complex.h>
#include "q15_to_cf.h"
#include "lora_fixed.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t n = 4096;
  lora_q15_complex *src = (lora_q15_complex *)aligned_alloc(32, n * sizeof(*src));
  float complex *dst = (float complex *)aligned_alloc(32, n * sizeof(*dst));
  if (!src || !dst) return 2;
  for (size_t i = 0; i < n; ++i) { src[i].r = (int16_t)(i & 0x7FFF); src[i].i = (int16_t)((i * 3) & 0x7FFF); }

  const int iters = 10000;
  struct timespec t0, t1; clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) q15_to_cf(dst, src, n);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double Msamples_per_s = ((double)n * iters) / (sec * 1e6);
  puts("metric,value");
  printf("q15_to_cf_Msamples_per_s,%.2f\n", Msamples_per_s);
  free(src); free(dst);
  return 0;
}

