#include "lora_fft_q15.h"
#include <string.h>

#if defined(LORA_LITE_USE_CMSIS) && defined(LORA_LITE_HAVE_CMSIS_HEADERS)
#include <arm_math.h>
static inline void cmsis_exec_q15(unsigned n, const q15_t *in, q15_t *out)
{
    /* CMSIS expects interleaved IQ arrays length 2N. */
    /* Copy input to output buffer then run in-place CFFT. */
    memcpy(out, in, 2u * n * sizeof(q15_t));
    arm_cfft_instance_q15 inst;
    if (arm_cfft_init_q15(&inst, n) == ARM_MATH_SUCCESS) {
        arm_cfft_q15(&inst, out, 0, 1);
    }
}
#endif

int lora_fft_q15_init(lora_fft_q15_ctx_t *ctx, unsigned n)
{
    if (!ctx || (n & (n - 1u)) != 0u) return -1; /* power of 2 */
    ctx->n = n;
#if defined(LORA_LITE_USE_CMSIS) && defined(LORA_LITE_HAVE_CMSIS_HEADERS)
    ctx->use_cmsis = 1;
#else
    ctx->use_cmsis = 0;
#endif
    ctx->opaque = NULL;
    return 0;
}

void lora_fft_q15_dispose(lora_fft_q15_ctx_t *ctx)
{
    (void)ctx;
}

void lora_fft_q15_exec_fwd(const lora_fft_q15_ctx_t *ctx,
                           const lora_q15_complex *restrict in,
                           lora_q15_complex *restrict out)
{
    unsigned n = ctx ? ctx->n : 0u;
    if (!n || !in || !out) return;

#if defined(LORA_LITE_USE_CMSIS) && defined(LORA_LITE_HAVE_CMSIS_HEADERS)
    /* Use CMSIS when available. Data is already interleaved in lora_q15_complex. */
    cmsis_exec_q15(n, (const q15_t *)in, (q15_t *)out);
#else
    /* Portable fallback: convert to float, run float FFT, convert back. */
    /* Note: This path exists for host builds and preserves behavior. */
    float complex tmp_in[n];
    float complex tmp_out[n];
    for (unsigned i = 0; i < n; ++i) tmp_in[i] = lora_q15_to_float(in[i]);
    /* Build a scratch FFT context on the stack using a tiny plan with caller-provided twiddles/work. */
    float complex work[n];
    float complex tw[n];
    lora_fft_ctx_t fctx;
    if (lora_fft_init(&fctx, n, work, tw, 0) != 0) return;
    lora_fft_exec_fwd(&fctx, tmp_in, tmp_out);
    lora_fft_dispose(&fctx);
    for (unsigned i = 0; i < n; ++i) {
        lora_q15_complex q;
        q.r = (q15)((int) (crealf(tmp_out[i]) * 32767.0f));
        q.i = (q15)((int) (cimagf(tmp_out[i]) * 32767.0f));
        out[i] = q;
    }
#endif
}

