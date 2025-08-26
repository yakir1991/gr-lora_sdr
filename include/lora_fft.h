#ifndef LORA_FFT_H
#define LORA_FFT_H

#include <complex.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned n;
    float complex *work;
    float complex *tw;
    int use_liquid;
    void *liquid_plan;
} lora_fft_ctx_t;

/* `work` and `tw` must both point to 32-byte aligned buffers of at least
 * `n` complex floats. */
int lora_fft_init(lora_fft_ctx_t *ctx, unsigned n,
                  float complex *work, float complex *tw, int use_liquid);
void lora_fft_dispose(lora_fft_ctx_t *ctx);
void lora_fft_exec_fwd(const lora_fft_ctx_t *ctx,
                       const float complex *restrict in,
                       float complex *restrict out);

#ifdef __cplusplus
}
#endif

#endif /* LORA_FFT_H */
