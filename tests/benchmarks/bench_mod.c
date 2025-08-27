#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <complex.h>
#include "lora_mod.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  const uint8_t sf = 7; const uint32_t bw = 125000; const uint32_t os = 8; const uint32_t fs = bw * os;
  const size_t nsym = 2048; /* large enough for timing */
  const uint32_t n_bins = 1u << sf; const uint32_t sps = n_bins * os;
  uint32_t *syms = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  float complex *chips = (float complex *)malloc(sizeof(float complex) * (size_t)sps * nsym);
  if (!syms || !chips) return 2;
  for (size_t i = 0; i < nsym; ++i) syms[i] = (uint32_t)(i % n_bins);

  struct timespec t0, t1; clock_gettime(CLOCK_MONOTONIC, &t0);
  lora_modulate(syms, chips, sf, fs, bw, nsym);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double us_per_sym = (sec * 1e6) / (double)nsym;
  puts("metric,value");
  printf("mod_us_per_sym_sf%u_os%u,%.3f\n", sf, os, us_per_sym);
  free(syms); free(chips); return 0;
}

