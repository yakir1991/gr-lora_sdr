// Benchmark demod throughput (symbols/s) to quantify no-copy FFT scan
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <complex.h>
#include <stdint.h>
#include "lora_fft_demod.h"

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(void) {
  puts("metric,value");
  for (uint8_t sf = 7; sf <= 12; ++sf) {
    uint32_t fs = 125000, bw = 125000; // OS=1 for simplicity
    uint32_t n_bins = 1u << sf;
    uint32_t sps = n_bins * (fs / bw);
    size_t nsym = 1024; // 1k symbols

    // Allocate chips: simple ideal upchirps for symbol 0 repeated
    float complex *chips = (float complex *)aligned_alloc(32, (size_t)sps * nsym * sizeof(float complex));
    float complex *up = (float complex *)aligned_alloc(32, sps * sizeof(float complex));
    float complex *down = (float complex *)aligned_alloc(32, sps * sizeof(float complex));
    if (!chips || !up || !down) return 2;
    lora_build_ref_chirps(up, down, sf, fs / bw);
    for (size_t s = 0; s < nsym; ++s) for (uint32_t k = 0; k < sps; ++k) chips[s*sps + k] = up[k];

    // Setup demod
    lora_fft_demod_ctx_t ctx;
    size_t ws_bytes = lora_fft_workspace_bytes(sf, fs, bw);
    void *ws = aligned_alloc(32, ws_bytes);
    if (!ws) return 3;
    if (lora_fft_demod_init(&ctx, sf, fs, bw, ws, ws_bytes) != 0) return 4;
    uint32_t *symbols = (uint32_t *)malloc(nsym * sizeof(uint32_t));
    if (!symbols) return 5;

    // Warm-up
    lora_fft_demod(&ctx, chips, nsym, symbols);

    // Benchmark
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    lora_fft_demod(&ctx, chips, nsym, symbols);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double sec = elapsed(t0, t1);
    double Msymps = (double)nsym / 1e6 / sec;
    printf("demod_Msymps_sf%u,%.3f\n", sf, Msymps);

    // Cleanup
    free(symbols); free(ws); free(up); free(down); free(chips);
  }
  return 0;
}

