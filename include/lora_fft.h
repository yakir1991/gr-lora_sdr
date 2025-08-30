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
    void *liquid_plan;      /* forward */
    void *liquid_plan_inv;  /* inverse */
} lora_fft_ctx_t;

/* `work` and `tw` must both point to 32-byte aligned buffers of at least
 * `n` complex floats. */
int lora_fft_init(lora_fft_ctx_t *ctx, unsigned n,
                  float complex *work, float complex *tw, int use_liquid);
void lora_fft_dispose(lora_fft_ctx_t *ctx);
void lora_fft_exec_fwd(const lora_fft_ctx_t *ctx,
                       const float complex *restrict in,
                       float complex *restrict out);
void lora_fft_exec_inv(const lora_fft_ctx_t *ctx,
                       const float complex *restrict in,
                       float complex *restrict out);

/* Helper: bytes required for FFT core workspace (work + twiddle).
 * Caller may over-provision for alignment.
 * Note: named 'core' to avoid clashing with demod workspace helper. */
static inline size_t lora_fft_core_workspace_bytes(unsigned n) {
    return 2u * (size_t)n * sizeof(float complex);
}

#ifdef __cplusplus
}
#endif

#endif /* LORA_FFT_H */
