#include "lora_fft_demod_ctx.h"
#include "lora_log.h"
#include "lora_utils.h"
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
  uint32_t fft_len = n_bins;

  size_t total = 0;
#ifndef LORA_LITE_LIQUID_FFT
  size_t cfg_sz = 0;
  kiss_fft_alloc(fft_len, 0, NULL, &cfg_sz);
  total = align_up(cfg_sz, alignof(lora_fft_cpx));
#endif
#ifndef LORA_LITE_FIXED_POINT
  total += sps * sizeof(float complex);
#else
  total += sps * sizeof(lora_q15_complex);
#endif
  total = align_up(total, alignof(lora_fft_cpx));
  total += fft_len * sizeof(lora_fft_cpx);
  total = align_up(total, alignof(lora_fft_cpx));
  total += fft_len * sizeof(lora_fft_cpx);
  return total;
}

int lora_fft_init(lora_fft_ctx_t *ctx, uint8_t sf, uint32_t fs, uint32_t bw,
                  void *workspace, size_t workspace_bytes) {
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
  uint32_t fft_len = n_bins;

  size_t need = lora_fft_workspace_bytes(sf, fs, bw);
  if (workspace_bytes < need)
    return -1;

  memset(ctx, 0, sizeof(*ctx));
  ctx->sf = sf;
  ctx->fs = fs;
  ctx->bw = bw;
  ctx->n_bins = n_bins;
  ctx->fft_len = fft_len;
  ctx->os_factor = os_factor;
  ctx->sps = sps;

  unsigned char *p =
      align_ptr((unsigned char *)workspace, alignof(lora_fft_cpx));

#ifndef LORA_LITE_LIQUID_FFT
  size_t cfg_sz = 0;
  kiss_fft_alloc(fft_len, 0, NULL, &cfg_sz);
  ctx->fft = kiss_fft_alloc(fft_len, 0, p, &cfg_sz);
  p += align_up(cfg_sz, alignof(lora_fft_cpx));
#endif

#ifndef LORA_LITE_FIXED_POINT
  ctx->downchirp = (float complex *)p;
  p += sps * sizeof(float complex);
  p = align_ptr(p, alignof(lora_fft_cpx));
#else
  ctx->downchirp = (lora_q15_complex *)p;
  p += sps * sizeof(lora_q15_complex);
  p = align_ptr(p, alignof(lora_fft_cpx));
#endif

  ctx->cx_in = (lora_fft_cpx *)p;
  p += fft_len * sizeof(lora_fft_cpx);
  p = align_ptr(p, alignof(lora_fft_cpx));
  ctx->cx_out = (lora_fft_cpx *)p;

#ifdef LORA_LITE_LIQUID_FFT
  ctx->fft =
      fft_create_plan(fft_len, ctx->cx_in, ctx->cx_out, LIQUID_FFT_FORWARD, 0);
#endif

#ifndef LORA_LITE_FIXED_POINT
  float complex upchirp[sps];
  lora_build_ref_chirps(upchirp, ctx->downchirp, sf, os_factor);
#else
  float complex upchirp_f[sps];
  float complex downchirp_f[sps];
  lora_build_ref_chirps(upchirp_f, downchirp_f, sf, os_factor);
  for (uint32_t n = 0; n < sps; ++n)
    ctx->downchirp[n] = lora_float_to_q15(downchirp_f[n]);
#endif

  ctx->cfo = 0.0f;
  ctx->cfo_phase = 0.0;
  return 0;
}

void lora_fft_destroy(lora_fft_ctx_t *ctx) {
#ifdef LORA_LITE_LIQUID_FFT
  fft_destroy_plan(ctx->fft);
#else
  (void)ctx;
  kiss_fft_cleanup();
#endif
}

void lora_fft_process(lora_fft_ctx_t *ctx, const float complex *restrict chips,
                      size_t nsym, uint32_t *restrict symbols) {
  if (!ctx || !chips || !symbols)
    return;

  uint32_t sps = ctx->sps;
  uint32_t n_bins = ctx->n_bins;
  uint32_t os_factor = ctx->os_factor;
  uint32_t fft_len = ctx->fft_len;

#ifndef LORA_LITE_FIXED_POINT
  float complex *restrict downchirp = ctx->downchirp;
#else
  lora_q15_complex *restrict downchirp = ctx->downchirp;
#endif
  lora_fft_cpx *restrict cx_in = ctx->cx_in;
  lora_fft_cpx *restrict cx_out = ctx->cx_out;

  if (ctx->cfo == 0.0f) {
    for (size_t s = 0; s < nsym; ++s) {
      const float complex *restrict symchips = chips + s * sps;
      for (uint32_t b = 0; b < n_bins; ++b) {
        float complex acc = 0.0f;
        uint32_t n = b * os_factor;
        for (uint32_t k = 0; k < os_factor; ++k, ++n) {
#ifndef LORA_LITE_FIXED_POINT
          acc += symchips[n] * downchirp[n];
#else
          lora_q15_complex cq = lora_float_to_q15(symchips[n]);
          lora_q15_complex m = lora_q15_mul(cq, downchirp[n]);
          acc += lora_q15_to_float(m);
#endif
        }
#ifdef LORA_LITE_LIQUID_FFT
        cx_in[b] = acc;
#else
        cx_in[b].r = crealf(acc);
        cx_in[b].i = cimagf(acc);
#endif
      }
#ifdef LORA_LITE_LIQUID_FFT
      fft_execute(ctx->fft);
#else
      kiss_fft(ctx->fft, cx_in, cx_out);
#endif

      float max_mag = 0.0f;
      uint32_t max_idx = 0;
      for (uint32_t i = 0; i < fft_len; ++i) {
        float mag;
#ifdef LORA_LITE_LIQUID_FFT
        float complex v = cx_out[i];
        mag = crealf(v) * crealf(v) + cimagf(v) * cimagf(v);
#else
        mag = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
#endif
        if (mag > max_mag) {
          max_mag = mag;
          max_idx = i;
        }
      }
      symbols[s] = max_idx & (ctx->n_bins - 1u);
    }
    return;
  }

  double dphi = -2.0 * M_PI * ctx->cfo / (double)ctx->fs;
  float complex phase = cexpf(I * (float)ctx->cfo_phase);
  float complex cfo_step = cexpf(I * (float)dphi);

  for (size_t s = 0; s < nsym; ++s) {
    const float complex *restrict symchips = chips + s * sps;
    for (uint32_t b = 0; b < n_bins; ++b) {
      float complex acc = 0.0f;
      uint32_t n = b * os_factor;
      for (uint32_t k = 0; k < os_factor; ++k, ++n) {
#ifndef LORA_LITE_FIXED_POINT
        float complex c = symchips[n] * downchirp[n];
#else
        lora_q15_complex cq = lora_float_to_q15(symchips[n]);
        lora_q15_complex m = lora_q15_mul(cq, downchirp[n]);
        float complex c = lora_q15_to_float(m);
#endif
        c *= phase;
        phase *= cfo_step;
        acc += c;
      }
#ifdef LORA_LITE_LIQUID_FFT
      cx_in[b] = acc;
#else
      cx_in[b].r = crealf(acc);
      cx_in[b].i = cimagf(acc);
#endif
    }
#ifdef LORA_LITE_LIQUID_FFT
    fft_execute(ctx->fft);
#else
    kiss_fft(ctx->fft, cx_in, cx_out);
#endif

    float max_mag = 0.0f;
    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < fft_len; ++i) {
      float mag;
#ifdef LORA_LITE_LIQUID_FFT
      float complex v = cx_out[i];
      mag = crealf(v) * crealf(v) + cimagf(v) * cimagf(v);
#else
      mag = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
#endif
      if (mag > max_mag) {
        max_mag = mag;
        max_idx = i;
      }
    }
    symbols[s] = max_idx & (ctx->n_bins - 1u);
  }

  ctx->cfo_phase += (double)nsym * (double)sps * dphi;
}
