// Micro-benchmark for Gray map/demap throughput
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "lora_graymap.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const size_t N = 1u << 16; // 65k symbols
  uint32_t *in = (uint32_t *)malloc(N * sizeof(uint32_t));
  uint32_t *tmp = (uint32_t *)malloc(N * sizeof(uint32_t));
  uint32_t *out = (uint32_t *)malloc(N * sizeof(uint32_t));
  if (!in || !tmp || !out) return 2;
  for (size_t i = 0; i < N; ++i) in[i] = (uint32_t)i;

  puts("metric,value");
  for (uint8_t sf = 7; sf <= 12; ++sf) {
    // Gray map
    unsigned iters = 200; // ~13M ops
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (unsigned r = 0; r < iters; ++r)
      lora_gray_map(in, tmp, sf, N);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double sec_map = elapsed(t0, t1);
    double Msymps_map = (double)N * (double)iters / 1e6 / sec_map;
    printf("gray_map_Msymps_sf%u,%.2f\n", sf, Msymps_map);

    // Gray demap
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (unsigned r = 0; r < iters; ++r)
      lora_gray_demap(tmp, out, sf, N);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double sec_demap = elapsed(t0, t1);
    double Msymps_demap = (double)N * (double)iters / 1e6 / sec_demap;
    printf("gray_demap_Msymps_sf%u,%.2f\n", sf, Msymps_demap);
  }

  free(in); free(tmp); free(out);
  return 0;
}

