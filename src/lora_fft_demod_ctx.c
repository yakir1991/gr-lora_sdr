#include "lora_fft_demod_ctx.h"
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
  uint32_t n_bins = 1u << sf;
  uint32_t os_factor = fs / bw;
  uint32_t sps = n_bins * os_factor;

  size_t cfg_sz = 0;
  kiss_fft_alloc(sps, 0, NULL, &cfg_sz);

  size_t total = align_up(cfg_sz, alignof(kiss_fft_cpx));
#ifndef LORA_LITE_FIXED_POINT
  total += sps * sizeof(float complex);
#else
  total += sps * sizeof(lora_q15_complex);
#endif
  total = align_up(total, alignof(kiss_fft_cpx));
  total += sps * sizeof(kiss_fft_cpx);
  total = align_up(total, alignof(kiss_fft_cpx));
  total += sps * sizeof(kiss_fft_cpx);
  return total;
}

int lora_fft_init(lora_fft_ctx_t *ctx, uint8_t sf, uint32_t fs, uint32_t bw,
                  void *workspace, size_t workspace_bytes) {
  if (!ctx || !workspace)
    return -1;

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
  ctx->os_factor = os_factor;
  ctx->sps = sps;

  unsigned char *p = align_ptr((unsigned char *)workspace, alignof(kiss_fft_cpx));

  size_t cfg_sz = 0;
  kiss_fft_alloc(sps, 0, NULL, &cfg_sz);
  ctx->fft = kiss_fft_alloc(sps, 0, p, &cfg_sz);
  p += align_up(cfg_sz, alignof(float complex));

#ifndef LORA_LITE_FIXED_POINT
  ctx->downchirp = (float complex *)p;
  p += sps * sizeof(float complex);
  p = align_ptr(p, alignof(kiss_fft_cpx));
#else
  ctx->downchirp = (lora_q15_complex *)p;
  p += sps * sizeof(lora_q15_complex);
  p = align_ptr(p, alignof(kiss_fft_cpx));
#endif

  ctx->cx_in = (kiss_fft_cpx *)p;
  p += sps * sizeof(kiss_fft_cpx);
  p = align_ptr(p, alignof(kiss_fft_cpx));
  ctx->cx_out = (kiss_fft_cpx *)p;

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
  (void)ctx;
  kiss_fft_cleanup();
}

void lora_fft_process(lora_fft_ctx_t *ctx, const float complex *chips,
                      size_t nsym, uint32_t *symbols) {
  if (!ctx || !chips || !symbols)
    return;

  uint32_t sps = ctx->sps;
  double phase = ctx->cfo_phase;
  double phase_inc = -2.0 * M_PI * ctx->cfo / (double)ctx->fs;

  for (size_t s = 0; s < nsym; ++s) {
    const float complex *symchips = &chips[s * sps];
    for (uint32_t n = 0; n < sps; ++n) {
#ifndef LORA_LITE_FIXED_POINT
      float complex c = symchips[n] * ctx->downchirp[n];
#else
      lora_q15_complex cq = lora_float_to_q15(symchips[n]);
      lora_q15_complex m = lora_q15_mul(cq, ctx->downchirp[n]);
      float complex cf = lora_q15_to_float(m);
#endif
      if (ctx->cfo != 0.0f) {
#ifndef LORA_LITE_FIXED_POINT
        c *= cexpf(I * (float)phase);
#else
        cf *= cexpf(I * (float)phase);
#endif
        phase += phase_inc;
      }
#ifndef LORA_LITE_FIXED_POINT
      ctx->cx_in[n].r = crealf(c);
      ctx->cx_in[n].i = cimagf(c);
#else
      ctx->cx_in[n].r = crealf(cf);
      ctx->cx_in[n].i = cimagf(cf);
#endif
    }
    kiss_fft(ctx->fft, ctx->cx_in, ctx->cx_out);

    float max_mag = 0.0f;
    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < sps; ++i) {
      float mag = ctx->cx_out[i].r * ctx->cx_out[i].r +
                  ctx->cx_out[i].i * ctx->cx_out[i].i;
      if (mag > max_mag) {
        max_mag = mag;
        max_idx = i;
      }
    }
    symbols[s] = (max_idx / ctx->os_factor) & (ctx->n_bins - 1u);
  }

  ctx->cfo_phase = phase;
}

