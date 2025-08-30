#ifndef LORA_FFT_Q15_H
#define LORA_FFT_Q15_H

#include <stddef.h>
#include "lora_fixed.h"
#include "lora_fft.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned n;
  int use_cmsis; /* runtime note only; compiled guards decide path */
  void *opaque;  /* reserved for CMSIS instance when available */
} lora_fft_q15_ctx_t;

int lora_fft_q15_init(lora_fft_q15_ctx_t *ctx, unsigned n);
void lora_fft_q15_dispose(lora_fft_q15_ctx_t *ctx);

/* Execute forward FFT on Q15 complex input; result in Q15 complex out.
 * When CMSIS is unavailable, falls back to float FFT via `lora_fft`. */
void lora_fft_q15_exec_fwd(const lora_fft_q15_ctx_t *ctx,
                           const lora_q15_complex *restrict in,
                           lora_q15_complex *restrict out);

#ifdef __cplusplus
}
#endif

#endif /* LORA_FFT_Q15_H */

