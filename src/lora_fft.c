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
    if (ctx->use_liquid && ctx->liquid_plan) {
#ifdef LORA_LITE_USE_LIQUID_FFT
        memcpy(ctx->work, in, n * sizeof(float complex));
        fft_execute((fftplan)ctx->liquid_plan);
        memcpy(out, ctx->work, n * sizeof(float complex));
        return;
#endif
    }

    memcpy(ctx->work, in, n * sizeof(float complex));
    bit_reverse(ctx->work, n);
    unsigned step = 1;
    while (step < n) {
        unsigned jump = step << 1;
        unsigned tw_step = n / jump;
        for (unsigned i = 0; i < n; i += jump) {
            for (unsigned k = 0; k < step; ++k) {
                float complex t = ctx->work[i + k + step] * ctx->tw[k * tw_step];
                float complex u = ctx->work[i + k];
                ctx->work[i + k] = u + t;
                ctx->work[i + k + step] = u - t;
            }
        }
        step = jump;
    }
    memcpy(out, ctx->work, n * sizeof(float complex));
}

