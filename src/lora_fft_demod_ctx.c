#ifdef LORA_LITE_FIXED_POINT
#include "lora_fft_demod_ctx.h"
#include "lora_log.h"
#include "lora_utils.h"
#include "q15_to_cf.h"
#include <math.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

static inline size_t align_up(size_t v, size_t a) {
  return (v + a - 1) & ~(a - 1);
}

static inline unsigned char *align_ptr(unsigned char *p, size_t a) {
  uintptr_t v = (uintptr_t)p;
  v = (v + a - 1) & ~(uintptr_t)(a - 1);
  return (unsigned char *)v;
}

size_t lora_fft_workspace_bytes(uint8_t sf, uint32_t fs, uint32_t bw) {
  uint32_t rem = fs % bw;
  if (rem) {
    LORA_LOG_WARN("fs %u not multiple of bw %u (rem %u)", fs, bw, rem);
    return 0;
  }
  uint32_t n_bins = 1u << sf;
  uint32_t os_factor = fs / bw;
  uint32_t sps = n_bins * os_factor;

  size_t total = 0;
  total = align_up(total, alignof(float complex));
  total += n_bins * sizeof(float complex); /* FFT work */
  total = align_up(total, alignof(float complex));
  total += n_bins * sizeof(float complex); /* FFT twiddles */
  total = align_up(total, alignof(float complex));
  total += n_bins * sizeof(float complex); /* FFT input */
  total = align_up(total, alignof(float complex));
  total += n_bins * sizeof(float complex); /* FFT output */
  total = align_up(total, alignof(float complex));
  total += sps * sizeof(float complex);    /* downchirp */
  return total;
}

int lora_fft_demod_init(lora_fft_demod_ctx_t *ctx, uint8_t sf, uint32_t fs,
                        uint32_t bw, void *workspace,
                        size_t workspace_bytes) {
  if (!ctx || !workspace)
    return -1;

  uint32_t rem = fs % bw;
  if (rem) {
    LORA_LOG_WARN("fs %u not multiple of bw %u (rem %u)", fs, bw, rem);
    return -1;
  }

  uint32_t n_bins = 1u << sf;
  uint32_t os_factor = fs / bw;
  uint32_t sps = n_bins * os_factor;
  size_t need = lora_fft_workspace_bytes(sf, fs, bw);
  if (workspace_bytes < need)
    return -1;

  memset(ctx, 0, sizeof(*ctx));
  ctx->sf = sf;
  ctx->fs = fs;
  ctx->bw = bw;
  ctx->n_bins = n_bins;
  ctx->fft_len = n_bins;
  ctx->os_factor = os_factor;
  ctx->sps = sps;

  unsigned char *p =
      align_ptr((unsigned char *)workspace, alignof(float complex));

  ctx->fft.work = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, alignof(float complex));

  ctx->fft.tw = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, alignof(float complex));

  ctx->fft_in = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, alignof(float complex));

  ctx->fft_out = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, alignof(float complex));

  ctx->downchirp = (float complex *)p;
  p += sps * sizeof(float complex);

  if (lora_fft_init(&ctx->fft, n_bins, ctx->fft.work, ctx->fft.tw, 0) != 0)
    return -1;

  float complex upchirp[sps];
  lora_build_ref_chirps(upchirp, ctx->downchirp, sf, os_factor);

  ctx->cfo = 0.0f;
  ctx->cfo_phase = 0.0;
  return 0;
}

void lora_fft_demod_destroy(lora_fft_demod_ctx_t *ctx) {
  lora_fft_dispose(&ctx->fft);
}

void lora_fft_process(lora_fft_demod_ctx_t *ctx,
                      const lora_q15_complex *restrict chips, size_t nsym,
                      uint32_t *restrict symbols) {
  if (!ctx || !chips || !symbols)
    return;

  uint32_t sps = ctx->sps;
  uint32_t n_bins = ctx->n_bins;
  uint32_t os_factor = ctx->os_factor;
  float complex *restrict downchirp = ctx->downchirp;
  float complex *restrict fft_in = ctx->fft_in;
  float complex *restrict fft_out = ctx->fft_out;

  _Alignas(32) float complex tmp[LORA_MAX_SPS];

  if (ctx->cfo == 0.0f) {
    for (size_t s = 0; s < nsym; ++s) {
      const lora_q15_complex *restrict symchips = chips + s * sps;
      q15_to_cf(tmp, symchips, sps);
      for (uint32_t b = 0; b < n_bins; ++b) {
        float complex acc = 0.0f;
        uint32_t n = b * os_factor;
        for (uint32_t k = 0; k < os_factor; ++k, ++n)
          acc += tmp[n] * downchirp[n];
        fft_in[b] = acc;
      }
      lora_fft_exec_fwd(&ctx->fft, fft_in, fft_out);
      float max_mag = 0.0f;
      uint32_t max_idx = 0;
      for (uint32_t i = 0; i < n_bins; ++i) {
        float complex v = fft_out[i];
        float mag = crealf(v) * crealf(v) + cimagf(v) * cimagf(v);
        if (mag > max_mag) {
          max_mag = mag;
          max_idx = i;
        }
      }
      symbols[s] = max_idx & (n_bins - 1u);
    }
    return;
  }

  double dphi = -2.0 * M_PI * ctx->cfo / (double)ctx->fs;
  float complex phase = cexpf(I * (float)ctx->cfo_phase);
  float complex step = cexpf(I * (float)dphi);

  for (size_t s = 0; s < nsym; ++s) {
    const lora_q15_complex *restrict symchips = chips + s * sps;
    q15_to_cf(tmp, symchips, sps);
    for (uint32_t b = 0; b < n_bins; ++b) {
      float complex acc = 0.0f;
      uint32_t n = b * os_factor;
      for (uint32_t k = 0; k < os_factor; ++k, ++n) {
        float complex c = tmp[n] * downchirp[n];
        c *= phase;
        phase *= step;
        acc += c;
      }
      fft_in[b] = acc;
    }
    lora_fft_exec_fwd(&ctx->fft, fft_in, fft_out);
    float max_mag = 0.0f;
    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < n_bins; ++i) {
      float complex v = fft_out[i];
      float mag = crealf(v) * crealf(v) + cimagf(v) * cimagf(v);
      if (mag > max_mag) {
        max_mag = mag;
        max_idx = i;
      }
    }
    symbols[s] = max_idx & (n_bins - 1u);
  }

  ctx->cfo_phase += (double)nsym * (double)sps * dphi;
}

#endif /* LORA_LITE_FIXED_POINT */
