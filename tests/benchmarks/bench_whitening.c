#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "lora_whitening.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t len = 8192;
  uint8_t *in = (uint8_t *)malloc(len);
  uint8_t *out = (uint8_t *)malloc(len);
  if (!in || !out) return 2;
  for (size_t i = 0; i < len; ++i) in[i] = (uint8_t)(i * 31u + 7u);

  struct timespec t0, t1;
  const int iters = 2000;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) {
    lora_whiten(in, out, len);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double mbps = ((double)len * iters) / (sec * 1e6);
  puts("metric,value");
  printf("whitening_MBps,%.3f\n", mbps);
  printf("bytes,%.0f\n", (double)len);
  free(in);
  free(out);
  return 0;
}

