#include "lora_fft.h"
#include <math.h>
#include <string.h>
#ifdef LORA_LITE_USE_LIQUID_FFT
#include <liquid/liquid.h>
#endif

int lora_fft_init(lora_fft_ctx_t *ctx, unsigned n,
                  float complex *work, float complex *tw, int use_liquid) {
    if (!ctx || !work || !tw)
        return -1;
    ctx->n = n;
    ctx->work = work;
    ctx->tw = tw;
    ctx->use_liquid = use_liquid;
    ctx->liquid_plan = NULL;
#ifdef LORA_LITE_USE_LIQUID_FFT
    if (use_liquid) {
        ctx->liquid_plan = fft_create_plan(n, work, work, LIQUID_FFT_FORWARD, 0);
        if (!ctx->liquid_plan)
            return -1;
    }
#endif
    for (unsigned i = 0; i < n; ++i) {
        double angle = -2.0 * M_PI * (double)i / (double)n;
        tw[i] = cexpf(I * (float)angle);
    }
    return 0;
}

void lora_fft_dispose(lora_fft_ctx_t *ctx) {
#ifdef LORA_LITE_USE_LIQUID_FFT
    if (ctx && ctx->use_liquid && ctx->liquid_plan)
        fft_destroy_plan((fftplan)ctx->liquid_plan);
#else
    (void)ctx;
#endif
}

static inline void bit_reverse(float complex *data, unsigned n) {
    unsigned j = 0;
    for (unsigned i = 0; i < n; ++i) {
        if (i < j) {
            float complex tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }
        unsigned bit = n >> 1;
        while (j & bit) {
            j &= ~bit;
            bit >>= 1;
        }
        j |= bit;
    }
}

void lora_fft_exec_fwd(const lora_fft_ctx_t *ctx,
                       const float complex *restrict in,
                       float complex *restrict out) {
    unsigned n = ctx->n;
    /* Create local restrict views and hint alignment for hot arrays. */
    float complex *restrict work = ctx->work;
    const float complex *restrict tw = ctx->tw;
#if defined(__GNUC__)
    work = (float complex *restrict)__builtin_assume_aligned(work, 32);
    tw   = (const float complex *restrict)__builtin_assume_aligned(tw, 32);
    in   = (const float complex *restrict)__builtin_assume_aligned(in, 32);
    out  = (float complex *restrict)__builtin_assume_aligned(out, 32);
#endif
    if (ctx->use_liquid && ctx->liquid_plan) {
#ifdef LORA_LITE_USE_LIQUID_FFT
        memcpy(work, in, n * sizeof(float complex));
        fft_execute((fftplan)ctx->liquid_plan);
        memcpy(out, work, n * sizeof(float complex));
        return;
#endif
    }

#if defined(LORA_LITE_FFT_BITREV_COPY)
    /* Copy input into work in bit-reversed order to avoid a separate pass. */
    unsigned log2n = 0; while ((1u << log2n) < n) ++log2n;
    for (unsigned i = 0; i < n; ++i) {
        unsigned x = i, r = 0;
        for (unsigned b = 0; b < log2n; ++b) { r = (r << 1) | (x & 1u); x >>= 1; }
        work[r] = in[i];
    }
#else
    /* Straight copy then in-place bit-reversal (copy-once mode off). */
    memcpy(work, in, n * sizeof(float complex));
    bit_reverse(work, n);
#endif

    unsigned step = 1;
    while (step < n) {
        unsigned jump = step << 1;
        unsigned tw_step = n / jump;
        for (unsigned i = 0; i < n; i += jump) {
            /* small unroll by 2 for inner butterflies when possible */
            unsigned k = 0;
            for (; k + 1 < step; k += 2) {
                float complex w0 = tw[(k + 0) * tw_step];
                float complex w1 = tw[(k + 1) * tw_step];
                float complex a0 = work[i + k + 0];
                float complex a1 = work[i + k + step + 0];
                float complex b0 = work[i + k + 1];
                float complex b1 = work[i + k + step + 1];
                float complex t0 = a1 * w0;
                float complex t1 = b1 * w1;
                work[i + k + 0]       = a0 + t0;
                work[i + k + step + 0] = a0 - t0;
                work[i + k + 1]       = b0 + t1;
                work[i + k + step + 1] = b0 - t1;
            }
            for (; k < step; ++k) {
                float complex t = work[i + k + step] * tw[k * tw_step];
                float complex u = work[i + k];
                work[i + k] = u + t;
                work[i + k + step] = u - t;
            }
        }
        step = jump;
    }
    /* Allow aliasing: if out == work, avoid final copy. */
    if (out != work) memcpy(out, work, n * sizeof(float complex));
}
