#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "lora_whitening.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void run_case(size_t len, int iters) {
  uint8_t *in = (uint8_t*)malloc(len);
  uint8_t *out = (uint8_t*)malloc(len);
  if (!in || !out) { puts("alloc_failed"); exit(2); }
  for (size_t i = 0; i < len; ++i) in[i] = (uint8_t)(i * 13u + 5u);
  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int r = 0; r < iters; ++r) lora_whiten(in, out, len);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double MBps = ((double)len * iters) / (sec * 1e6);
  printf("whiten_MBps_len%zu,%.2f\n", len, MBps);
  free(in); free(out);
}

int main(void) {
  puts("metric,value");
  run_case(256,   500000);
  run_case(1024,  200000);
  run_case(8192,   30000);
  return 0;
}

