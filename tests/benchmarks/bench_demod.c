#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <complex.h>
#include <stdint.h>
#include "lora_fft_demod.h"
#include "lora_utils.h"
#include "lora_config.h"

#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#endif

static double elapsed(struct timespec a, struct timespec b) {
  return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

static void *aligned_alloc32(size_t nbytes) {
  void *p = NULL;
  if (posix_memalign(&p, 32, nbytes) != 0) return NULL;
  return p;
}

int main(int argc, char **argv) {
  (void)argc; (void)argv;

  const uint8_t sf = 7;          // small, representative
  const uint32_t bw = 125000;    // 125 kHz
  const uint32_t os = 8;         // oversample factor
  const uint32_t fs = bw * os;   // sample rate
  const size_t nsym = 128;       // number of symbols to demod

  const uint32_t n_bins = 1u << sf;
  const uint32_t sps = n_bins * os;
  const size_t chips_len = (size_t)sps * nsym;

  // Prepare reference chirps and chips buffer
  float complex *up = (float complex *)aligned_alloc32(sizeof(float complex) * sps);
  float complex *down = (float complex *)aligned_alloc32(sizeof(float complex) * sps);
  if (!up || !down) return 2;
  lora_build_ref_chirps(up, down, sf, os);

#ifdef LORA_LITE_FIXED_POINT
  lora_q15_complex *chips = (lora_q15_complex *)malloc(sizeof(lora_q15_complex) * chips_len);
  if (!chips) return 3;
  const float scale = 32767.0f;
  for (size_t s = 0; s < nsym; ++s) {
    // use symbol ID = s % n_bins to vary
    uint32_t id = (uint32_t)(s % n_bins);
    // copy an upchirp rotated by id into chips (approximate payload)
    for (uint32_t n = 0; n < sps; ++n) {
      float complex v = up[n];
      float re = crealf(v), im = cimagf(v);
      int16_t qr = (int16_t)(re * scale + (re >= 0 ? 0.5f : -0.5f));
      int16_t qi = (int16_t)(im * scale + (im >= 0 ? 0.5f : -0.5f));
      chips[s * sps + n].r = qr;
      chips[s * sps + n].i = qi;
    }
    (void)id; // symbol value not used directly
  }
#else
  float complex *chips = (float complex *)malloc(sizeof(float complex) * chips_len);
  if (!chips) return 3;
  for (size_t s = 0; s < nsym; ++s) {
    memcpy(&chips[s * sps], up, sizeof(float complex) * sps);
  }
#endif

  // Workspace
  size_t need = lora_fft_workspace_bytes(sf, fs, bw);
  void *ws = aligned_alloc32(need);
  if (!ws) return 4;

  lora_fft_demod_ctx_t ctx;
  if (lora_fft_demod_init(&ctx, sf, fs, bw, ws, need) != 0) return 5;

  uint32_t *symbols = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!symbols) return 6;

  // Time demod of nsym symbols
  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  lora_fft_demod(&ctx, chips, nsym, symbols);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double sec = elapsed(t0, t1);
  double us_per_sym = (sec * 1e6) / (double)nsym;

  puts("metric,value");
  printf("demod_us_per_sym_sf%u_os%u,%.3f\n", sf, os, us_per_sym);
  printf("demod_workspace_bytes,%.0f\n", (double)need);

  // Cleanup
  lora_fft_demod_free(&ctx);
  free(symbols);
  free(ws);
  free(chips);
  free(up);
  free(down);
  return 0;
}

