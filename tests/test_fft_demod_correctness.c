#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>

#include "lora_fft_demod.h"
#include "lora_utils.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#endif

static void *aligned_alloc32(size_t nbytes) {
  void *p = NULL;
  if (posix_memalign(&p, 32, nbytes) != 0) return NULL;
  return p;
}

int main(void) {
  const uint8_t sf = 7;
  const uint32_t bw = 125000;
  const uint32_t os = 8;
  const uint32_t fs = bw * os;
  const size_t nsym = 8;

  const uint32_t n_bins = 1u << sf;
  const uint32_t sps = n_bins * os;
  const size_t chips_len = (size_t)sps * nsym;

  float complex *up = (float complex *)aligned_alloc32(sizeof(float complex) * sps);
  float complex *down = (float complex *)aligned_alloc32(sizeof(float complex) * sps);
  if (!up || !down) return 2;
  lora_build_ref_chirps(up, down, sf, os);

  // Build chips: nsym copies of plain upchirp => expect symbol 0
#ifdef LORA_LITE_FIXED_POINT
  lora_q15_complex *chips = (lora_q15_complex *)malloc(sizeof(lora_q15_complex) * chips_len);
  if (!chips) return 3;
  const float scale = 32767.0f;
  for (size_t s = 0; s < nsym; ++s) {
    for (uint32_t n = 0; n < sps; ++n) {
      float re = crealf(up[n]);
      float im = cimagf(up[n]);
      int16_t qr = (int16_t)(re * scale + (re >= 0 ? 0.5f : -0.5f));
      int16_t qi = (int16_t)(im * scale + (im >= 0 ? 0.5f : -0.5f));
      chips[s * sps + n].r = qr;
      chips[s * sps + n].i = qi;
    }
  }
#else
  float complex *chips = (float complex *)malloc(sizeof(float complex) * chips_len);
  if (!chips) return 3;
  for (size_t s = 0; s < nsym; ++s)
    memcpy(&chips[s * sps], up, sizeof(float complex) * sps);
#endif

  size_t need = lora_fft_workspace_bytes(sf, fs, bw);
  void *ws = aligned_alloc32(need);
  if (!ws) return 4;

  lora_fft_demod_ctx_t ctx;
  if (lora_fft_demod_init(&ctx, sf, fs, bw, ws, need) != 0) return 5;

  uint32_t *symbols = (uint32_t *)malloc(sizeof(uint32_t) * nsym);
  if (!symbols) return 6;

  lora_fft_demod(&ctx, chips, nsym, symbols);

  int ok = 1;
  for (size_t i = 0; i < nsym; ++i) {
    if (symbols[i] != 0u) { ok = 0; break; }
  }

  if (ok) {
    puts("FFT demod correctness test passed");
  } else {
    fprintf(stderr, "Mismatch: expected all zeros, got [%u, ...]\n", symbols[0]);
    return 10;
  }

  lora_fft_demod_free(&ctx);
  free(symbols);
  free(ws);
  free(chips);
  free(up);
  free(down);
  return 0;
}
