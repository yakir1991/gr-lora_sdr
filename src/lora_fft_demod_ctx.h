#ifndef LORA_FFT_DEMOD_CTX_H
#define LORA_FFT_DEMOD_CTX_H

#include "lora_config.h"
#include "lora_fixed.h"
#include <complex.h>
#ifdef LORA_LITE_LIQUID_FFT
#include <liquid/liquid.h>
typedef fftplan lora_fft_plan;
typedef float complex lora_fft_cpx;
#else
#include <kiss_fft.h>
typedef kiss_fft_cfg lora_fft_plan;
typedef kiss_fft_cpx lora_fft_cpx;
#endif
#include <stddef.h>
#include <stdint.h>

/* Context for the FFT-based LoRa demodulator.  All state required for
 * processing is stored here so that runtime operation performs no dynamic
 * allocation. */
typedef struct {
  uint8_t sf;         /* spreading factor */
  uint32_t fs;        /* sample rate */
  uint32_t bw;        /* signal bandwidth */
  uint32_t n_bins;    /* number of FFT bins */
  uint32_t fft_len;   /* FFT length (same as n_bins) */
  uint32_t os_factor; /* oversampling factor */
  uint32_t sps;       /* samples per symbol */

  float cfo;        /* carrier frequency offset in Hz */
  double cfo_phase; /* accumulated CFO phase */

#ifndef LORA_LITE_FIXED_POINT
  float complex *downchirp; /* precomputed downchirp */
#else
  lora_q15_complex *downchirp; /* precomputed downchirp */
#endif

  lora_fft_plan fft;    /* FFT plan */
  lora_fft_cpx *cx_in;  /* FFT input buffer */
  lora_fft_cpx *cx_out; /* FFT output buffer */
} lora_fft_ctx_t;

/* Return the number of bytes required for the workspace used by the
 * demodulator with the given parameters. */
size_t lora_fft_workspace_bytes(uint8_t sf, uint32_t fs, uint32_t bw);

/* Initialise the context using the caller supplied workspace.  The workspace
 * must be at least lora_fft_workspace_bytes(sf,fs,bw) bytes. */
int lora_fft_init(lora_fft_ctx_t *ctx, uint8_t sf, uint32_t fs, uint32_t bw,
                  void *workspace, size_t workspace_bytes);

/* Release any resources held by the context.  The workspace memory itself is
 * owned by the caller and is not freed. */
void lora_fft_destroy(lora_fft_ctx_t *ctx);

/* Demodulate nsym symbols from the chips array and store the recovered symbol
 * indices in the symbols array.  All working buffers are taken from the
 * context and no dynamic allocation occurs. */
void lora_fft_process(lora_fft_ctx_t *ctx, const float complex *restrict chips,
                      size_t nsym, uint32_t *restrict symbols);

#endif /* LORA_FFT_DEMOD_CTX_H */
