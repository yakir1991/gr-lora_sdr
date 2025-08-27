#ifndef LORA_FFT_DEMOD_H
#define LORA_FFT_DEMOD_H

#include "lora_config.h"
#include "lora_fixed.h"
#include "lora_fft.h"
#include <stddef.h>
#include <complex.h>
#include <stdint.h>

/* Context for the FFT-based LoRa demodulator.  All state required for
 * processing is stored here so that runtime operation performs no dynamic
 * allocation. */
typedef struct lora_fft_demod_ctx {
  uint8_t sf;         /* spreading factor */
  uint32_t fs;        /* sample rate */
  uint32_t bw;        /* signal bandwidth */
  uint32_t n_bins;    /* number of FFT bins */
  uint32_t fft_len;   /* FFT length (same as n_bins) */
  uint32_t os_factor; /* oversampling factor */
  uint32_t sps;       /* samples per symbol */

  float cfo;        /* carrier frequency offset in Hz */
  double cfo_phase; /* accumulated CFO phase */

  _Alignas(32) float complex *downchirp; /* precomputed downchirp */
  _Alignas(32) float complex *fft_in;    /* FFT input buffer */
  _Alignas(32) float complex *fft_out;   /* FFT output buffer */
  lora_fft_ctx_t fft;                    /* FFT context */
#ifdef LORA_LITE_FIXED_POINT
  _Alignas(32) lora_q15_complex *downchirp_q15; /* Q15 downchirp (for fixed path) */
  _Alignas(32) lora_q15_complex *bins_q15;      /* Q15 accumulation buffer (n_bins) */
#endif
} lora_fft_demod_ctx_t;

/* Return the number of bytes required for the workspace used by the
 * demodulator with the given parameters.  The returned size is always a
 * multiple of 32 bytes so it can be passed directly to aligned_alloc. */
size_t lora_fft_workspace_bytes(uint8_t sf, uint32_t fs, uint32_t bw);

/* Initialise the context using the caller supplied workspace.  The workspace
 * must be 32-byte aligned and at least lora_fft_workspace_bytes(sf,fs,bw)
 * bytes. */
int lora_fft_demod_init(lora_fft_demod_ctx_t *ctx, uint8_t sf, uint32_t fs,
                        uint32_t bw, void *workspace,
                        size_t workspace_bytes);

/* Release any resources held by the context.  The workspace memory itself is
 * owned by the caller and is not freed. */
void lora_fft_demod_free(lora_fft_demod_ctx_t *ctx);

/* Demodulate nsym symbols from the chips array and store the recovered symbol
 * indices in the symbols array.  All working buffers are taken from the
 * context and no dynamic allocation occurs. */
void lora_fft_demod(lora_fft_demod_ctx_t *ctx,
#ifdef LORA_LITE_FIXED_POINT
                      const lora_q15_complex *restrict chips,
#else
                      const float complex *restrict chips,
#endif
                      size_t nsym,
                      uint32_t *restrict symbols);

#endif /* LORA_FFT_DEMOD_H */
