#include "lora_fft_demod.h"
#include "lora_log.h"
#include "lora_utils.h"
#ifdef LORA_LITE_FIXED_POINT
#include "q15_to_cf.h"
#endif
#include <math.h>
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

static inline int16_t sat16(int32_t x) {
  if (x > 32767) return 32767;
  if (x < -32768) return -32768;
  return (int16_t)x;
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

  const size_t align = 32;
  size_t total = 0;
  total = align_up(total, align);
  total += n_bins * sizeof(float complex); /* FFT work */
  total = align_up(total, align);
  total += n_bins * sizeof(float complex); /* FFT twiddles */
  total = align_up(total, align);
  total += n_bins * sizeof(float complex); /* FFT input */
  total = align_up(total, align);
  total += n_bins * sizeof(float complex); /* FFT output */
  total = align_up(total, align);
  total += sps * sizeof(float complex); /* downchirp (float) */
  total = align_up(total, align);
#ifdef LORA_LITE_FIXED_POINT
  total += sps * sizeof(lora_q15_complex); /* downchirp (q15) */
  total = align_up(total, align);
  total += n_bins * sizeof(lora_q15_complex); /* bins_q15 */
  total = align_up(total, align);
#endif
  return total;
}

int lora_fft_demod_init(lora_fft_demod_ctx_t *ctx, uint8_t sf, uint32_t fs,
                        uint32_t bw, void *workspace, size_t workspace_bytes) {
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

  const size_t align = 32;
  unsigned char *p = align_ptr((unsigned char *)workspace, align);

  ctx->fft.work = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, align);

  ctx->fft.tw = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, align);

  ctx->fft_in = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, align);

  ctx->fft_out = (float complex *)p;
  p += n_bins * sizeof(float complex);
  p = align_ptr(p, align);

  ctx->downchirp = (float complex *)p;
  p += sps * sizeof(float complex);

#ifdef LORA_LITE_FIXED_POINT
  p = align_ptr(p, align);
  ctx->downchirp_q15 = (lora_q15_complex *)p;
  p += sps * sizeof(lora_q15_complex);
  p = align_ptr(p, align);
  ctx->bins_q15 = (lora_q15_complex *)p;
  p += n_bins * sizeof(lora_q15_complex);
#endif

  int use_liquid_fft = 0;
#ifdef LORA_LITE_USE_LIQUID_FFT
  use_liquid_fft = 1;
#endif
  if (lora_fft_init(&ctx->fft, n_bins, ctx->fft.work, ctx->fft.tw, use_liquid_fft) != 0)
    return -1;

#if defined(LORA_LITE_FIXED_POINT) && defined(LORA_LITE_USE_CMSIS)
  (void)lora_fft_q15_init(&ctx->fft_q15, n_bins);
#endif

  float complex upchirp[sps];
  lora_build_ref_chirps(upchirp, ctx->downchirp, sf, os_factor);

#ifdef LORA_LITE_FIXED_POINT
  /* Quantize downchirp to Q15 once to avoid per-symbol q15->float conversion */
  const float scale = 32767.0f;
  for (uint32_t n = 0; n < sps; ++n) {
    float re = crealf(ctx->downchirp[n]);
    float im = cimagf(ctx->downchirp[n]);
    int16_t qr = (int16_t)(re * scale + (re >= 0 ? 0.5f : -0.5f));
    int16_t qi = (int16_t)(im * scale + (im >= 0 ? 0.5f : -0.5f));
    ctx->downchirp_q15[n].r = qr;
    ctx->downchirp_q15[n].i = qi;
  }
#endif

  ctx->cfo = 0.0f;
  ctx->cfo_phase = 0.0;
  return 0;
}

void lora_fft_demod_free(lora_fft_demod_ctx_t *ctx) {
  lora_fft_dispose(&ctx->fft);
#if defined(LORA_LITE_FIXED_POINT) && defined(LORA_LITE_USE_CMSIS)
  lora_fft_q15_dispose(&ctx->fft_q15);
#endif
}

/*
 * Mix the incoming chips with the pre-computed downchirp and accumulate
 * results into the FFT input buffer. When `apply_cfo` is non-zero, each
 * sample is additionally rotated by the running CFO phase.
 */
static inline void dechirp_and_accumulate(
    const float complex *restrict sc, const float complex *restrict downchirp,
    uint32_t n_bins, uint32_t os_factor, float complex *restrict fft_in,
    int apply_cfo, float complex *phase, float complex step) {
  for (uint32_t b = 0; b < n_bins; ++b) {
    float complex acc = 0.0f;
    uint32_t n = b * os_factor;
    for (uint32_t k = 0; k < os_factor; ++k, ++n) {
      float complex c = sc[n] * downchirp[n];
      if (apply_cfo) {
        c *= *phase;
        *phase *= step;
      }
      acc += c;
    }
    fft_in[b] = acc;
  }
}

void lora_fft_demod(lora_fft_demod_ctx_t *ctx,
#ifdef LORA_LITE_FIXED_POINT
                    const lora_q15_complex *restrict chips,
#else
                    const float complex *restrict chips,
#endif
                    size_t nsym, uint32_t *restrict symbols) {
  if (!ctx || !chips || !symbols)
    return;

  uint32_t sps = ctx->sps;
  uint32_t n_bins = ctx->n_bins;
  uint32_t os_factor = ctx->os_factor;
  float complex *restrict downchirp = ctx->downchirp;
  float complex *restrict fft_in = ctx->fft_in;
  float complex *restrict fft_out = ctx->fft_out; /* legacy alias; may bypass below */

#ifdef LORA_LITE_FIXED_POINT
  _Alignas(32) float complex tmp[LORA_MAX_SPS];
#endif

  if (ctx->cfo == 0.0f) {
    for (size_t s = 0; s < nsym; ++s) {
#ifdef LORA_LITE_FIXED_POINT
      const lora_q15_complex *restrict symchips = chips + s * sps;
      /* Fixed-point fast path: accumulate in Q15 per FFT bin, then convert n_bins only */
      for (uint32_t b = 0; b < n_bins; ++b) {
        int32_t acc_r = 0, acc_i = 0;
        uint32_t n = b * os_factor;
        for (uint32_t k = 0; k < os_factor; ++k, ++n) {
          /* acc += symchips[n] * downchirp_q15[n]; */
          lora_q15_complex prod;
          /* inline multiply to keep dependency on lora_fixed minimal */
          int32_t ar = symchips[n].r;
          int32_t ai = symchips[n].i;
          int32_t br = ctx->downchirp_q15[n].r;
          int32_t bi = ctx->downchirp_q15[n].i;
          int32_t pr = (ar * br - ai * bi) >> 15;
          int32_t pi = (ar * bi + ai * br) >> 15;
          prod.r = (int16_t)pr;
          prod.i = (int16_t)pi;
          acc_r += prod.r;
          acc_i += prod.i;
        }
        ctx->bins_q15[b].r = sat16(acc_r);
        ctx->bins_q15[b].i = sat16(acc_i);
      }
      /* Convert only n_bins complex values to float for FFT input */
      q15_to_cf(fft_in, ctx->bins_q15, n_bins);
#else
      const float complex *restrict sc = chips + s * sps;
      /* No CFO: helper skips phase rotation. */
      dechirp_and_accumulate(sc, downchirp, n_bins, os_factor, fft_in, 0, NULL,
                             0.0f);
#endif
      /* Choose FFT backend depending on build flags */
#if defined(LORA_LITE_FIXED_POINT) && defined(LORA_LITE_USE_CMSIS)
      lora_fft_q15_exec_fwd(&ctx->fft_q15, ctx->bins_q15, ctx->bins_q15);
      /* Convert FFT output to float for magnitude search */
      q15_to_cf(fft_out, ctx->bins_q15, n_bins);
#else
      /* Avoid final copy inside FFT by writing into ctx->fft.work directly */
      lora_fft_exec_fwd(&ctx->fft, fft_in, ctx->fft.work);
#endif
      const float complex *restrict spec = ctx->fft.work;
      float max_mag = 0.0f;
      uint32_t max_idx = 0;
      for (uint32_t i = 0; i < n_bins; ++i) {
        float complex v = spec[i];
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
#ifdef LORA_LITE_FIXED_POINT
    const lora_q15_complex *restrict symchips = chips + s * sps;
    /* Fixed-point CFO path: dechirp and CFO-rotate in Q15, accumulate per-bin.
       Keep phase continuous across all samples in the symbol. */
    float complex ph = phase;
    for (uint32_t b = 0; b < n_bins; ++b) {
      int32_t acc_r = 0, acc_i = 0;
      uint32_t n = b * os_factor;
      for (uint32_t k = 0; k < os_factor; ++k, ++n) {
        /* Compute phasor in Q15 for this sample */
        float pr = crealf(ph), pi = cimagf(ph);
        int16_t qpr = (int16_t)(pr * 32767.0f + (pr >= 0 ? 0.5f : -0.5f));
        int16_t qpi = (int16_t)(pi * 32767.0f + (pi >= 0 ? 0.5f : -0.5f));
        /* tmp = sym[n] * down[n] */
        int32_t ar = symchips[n].r, ai = symchips[n].i;
        int32_t br = ctx->downchirp_q15[n].r, bi = ctx->downchirp_q15[n].i;
        int32_t tr = (ar * br - ai * bi) >> 15;
        int32_t ti = (ar * bi + ai * br) >> 15;
        /* apply CFO: tmp *= phasor */
        int32_t cr = (tr * qpr - ti * qpi) >> 15;
        int32_t ci = (tr * qpi + ti * qpr) >> 15;
        acc_r += cr;
        acc_i += ci;
        ph *= step;
      }
      ctx->bins_q15[b].r = sat16(acc_r);
      ctx->bins_q15[b].i = sat16(acc_i);
    }
    /* Convert accumulated bins to float for FFT */
    q15_to_cf(fft_in, ctx->bins_q15, n_bins);
    /* Advance global phase to end-of-symbol */
    phase = ph;
#else
    const float complex *restrict sc = chips + s * sps;
#endif
    /* CFO present: rotate each chip by the evolving phase. */
#if !defined(LORA_LITE_FIXED_POINT)
    dechirp_and_accumulate(sc, downchirp, n_bins, os_factor, fft_in, 1, &phase, step);
    lora_fft_exec_fwd(&ctx->fft, fft_in, ctx->fft.work);
#else
    /* Fixed-point path already filled ctx->bins_q15 with dechirped accumulations. */
#  if defined(LORA_LITE_USE_CMSIS)
    lora_fft_q15_exec_fwd(&ctx->fft_q15, ctx->bins_q15, ctx->bins_q15);
    q15_to_cf(fft_out, ctx->bins_q15, n_bins);
#  else
    /* Fallback: convert to float then run float FFT */
    q15_to_cf(fft_in, ctx->bins_q15, n_bins);
    lora_fft_exec_fwd(&ctx->fft, fft_in, ctx->fft.work);
#  endif
#endif
    const float complex *restrict spec = ctx->fft.work;
    float max_mag = 0.0f;
    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < n_bins; ++i) {
      float complex v = spec[i];
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
