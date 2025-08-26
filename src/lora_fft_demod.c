#include "lora_fft_demod.h"
#include "lora_fft_demod_ctx.h"
#include <stdlib.h>

/* Thin wrapper that preserves the historic API while internally using the
 * context based FFT demodulator.  A temporary workspace is allocated for each
 * call. */
void lora_fft_demod(const lora_q15_complex *restrict chips,
                    uint32_t *restrict symbols, uint8_t sf,
                    uint32_t samp_rate, uint32_t bw,
                    float freq_offset, size_t nsym) {
  lora_fft_demod_ctx_t ctx;
  size_t ws_bytes = lora_fft_workspace_bytes(sf, samp_rate, bw);
  if (ws_bytes == 0)
    return;

  void *ws = malloc(ws_bytes);
  if (!ws)
    return;

  if (lora_fft_demod_init(&ctx, sf, samp_rate, bw, ws, ws_bytes) != 0) {
    free(ws);
    return;
  }

  ctx.cfo = freq_offset;
  ctx.cfo_phase = 0.0;
  lora_fft_process(&ctx, chips, nsym, symbols);
  lora_fft_demod_destroy(&ctx);
  free(ws);
}

