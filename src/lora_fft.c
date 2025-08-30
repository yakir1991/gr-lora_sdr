#include "lora_fft.h"
#include <math.h>
#include <string.h>
#if defined(LORA_LITE_NEON) && defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#ifdef LORA_LITE_USE_LIQUID_FFT
#include <liquid/liquid.h>
#endif

int lora_fft_init(lora_fft_ctx_t *ctx, unsigned n,
                  float complex *work, float complex *tw, int use_liquid) {
    if (!ctx || !work || !tw)
        return -1;
    /* Require power-of-two length */
    if ((n & (n - 1u)) != 0u) return -1;
    ctx->n = n;
    ctx->work = work;
    ctx->tw = tw;
    ctx->use_liquid = use_liquid;
    ctx->liquid_plan = NULL;
    ctx->liquid_plan_inv = NULL;
#ifdef LORA_LITE_USE_LIQUID_FFT
    if (use_liquid) {
        ctx->liquid_plan = fft_create_plan(n, work, work, LIQUID_FFT_FORWARD, 0);
        if (!ctx->liquid_plan)
            return -1;
        ctx->liquid_plan_inv = fft_create_plan(n, work, work, LIQUID_FFT_BACKWARD, 0);
        if (!ctx->liquid_plan_inv)
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
    if (ctx && ctx->use_liquid) {
        if (ctx->liquid_plan)
            fft_destroy_plan((fftplan)ctx->liquid_plan);
        if (ctx->liquid_plan_inv)
            fft_destroy_plan((fftplan)ctx->liquid_plan_inv);
    }
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

/* Fast bit-reverse index using 8-bit reversal table. Avoids per-bit loop. */
static inline unsigned bitrev_index(unsigned x, unsigned log2n) {
#if defined(__has_builtin)
#  if __has_builtin(__builtin_bitreverse32)
    return __builtin_bitreverse32(x) >> (32u - log2n);
#  endif
#endif
    static const unsigned char rev8[256] = {
        0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
        0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
        0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
        0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
        0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
        0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
        0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
        0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
        0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
        0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
        0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
        0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
        0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
        0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
        0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
        0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
    };
    unsigned b0 = rev8[(x >>  0) & 0xFFu];
    unsigned b1 = rev8[(x >>  8) & 0xFFu];
    unsigned b2 = rev8[(x >> 16) & 0xFFu];
    unsigned b3 = rev8[(x >> 24) & 0xFFu];
    unsigned r  = (b0 << 24) | (b1 << 16) | (b2 << 8) | (b3 << 0);
    return r >> (32u - log2n);
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
    /* Copy input into work in bit-reversed order to avoid a separate pass.
     * Use table-accelerated bit-reverse to minimize scalar loops. */
    unsigned log2n = 0; while ((1u << log2n) < n) ++log2n;
    for (unsigned i = 0; i < n; ++i) {
        unsigned r = bitrev_index(i, log2n);
        work[r] = in[i];
    }
#else
    /* Straight copy then in-place bit-reversal (copy-once mode off). */
    memcpy(work, in, n * sizeof(float complex));
    bit_reverse(work, n);
#endif

    unsigned step = 1;
#if defined(LORA_LITE_FFT_RADIX4)
    /* Radix-4 passes when possible (combine two radix-2 stages). */
    while ((step << 2) <= n) {
        unsigned jump = step << 2;           /* group size */
        unsigned tw_step = n / jump;         /* base twiddle stride */
        for (unsigned i = 0; i < n; i += jump) {
            for (unsigned k = 0; k < step; ++k) {
                float complex w1 = tw[(k * 1u) * tw_step];
                float complex w2 = tw[(k * 2u) * tw_step];
                float complex w3 = tw[(k * 3u) * tw_step];
                float complex a0 = work[i + k + 0*step];
                float complex a1 = work[i + k + 1*step] * w1;
                float complex a2 = work[i + k + 2*step] * w2;
                float complex a3 = work[i + k + 3*step] * w3;
                float complex t0 = a0 + a2;
                float complex t1 = a0 - a2;
                float complex t2 = a1 + a3;
                float complex t3 = (a1 - a3) * (-I);
                work[i + k + 0*step] = t0 + t2;
                work[i + k + 1*step] = t1 + t3;
                work[i + k + 2*step] = t0 - t2;
                work[i + k + 3*step] = t1 - t3;
            }
        }
        step = jump;
    }
#endif
    while (step < n) {
        unsigned jump = step << 1;
        unsigned tw_step = n / jump;
        for (unsigned i = 0; i < n; i += jump) {
#if defined(LORA_LITE_TWIDDLE_RECURRENCE)
            float complex w_inc = tw[tw_step];       /* base twiddle increment */
            float complex w_inc2 = w_inc * w_inc;    /* for unroll by 2 */
            float complex wk = 1.0f;
            unsigned k = 0;
            for (; k + 1 < step; k += 2) {
#if defined(LORA_LITE_EMB_THROUGHPUT) && (defined(__arm__) || defined(__aarch64__))
                __builtin_prefetch(&work[i + k + 8], 0, 1);
                __builtin_prefetch(&work[i + k + step + 8], 0, 1);
#endif
                float complex w0 = wk;
                float complex w1 = wk * w_inc;
                float complex a0 = work[i + k + 0];
                float complex a1 = work[i + k + step + 0];
                float complex b0 = work[i + k + 1];
                float complex b1 = work[i + k + step + 1];
#if defined(LORA_LITE_NEON) && defined(__ARM_NEON)
                float32x2_t a1v = vld1_f32((const float*)&a1);
                float32x2_t w0v = vld1_f32((const float*)&w0);
                float32x2_t b1v = vld1_f32((const float*)&b1);
                float32x2_t w1v = vld1_f32((const float*)&w1);
                float32x2_t t0v = { a1v[0]*w0v[0] - a1v[1]*w0v[1], a1v[0]*w0v[1] + a1v[1]*w0v[0] };
                float32x2_t t1v = { b1v[0]*w1v[0] - b1v[1]*w1v[1], b1v[0]*w1v[1] + b1v[1]*w1v[0] };
                float complex t0, t1;
                vst1_f32((float*)&t0, t0v);
                vst1_f32((float*)&t1, t1v);
#else
                float complex t0 = a1 * w0;
                float complex t1 = b1 * w1;
#endif
                work[i + k + 0]        = a0 + t0;
                work[i + k + step + 0] = a0 - t0;
                work[i + k + 1]        = b0 + t1;
                work[i + k + step + 1] = b0 - t1;
                wk *= w_inc2; /* advance by 2 */
            }
            for (; k < step; ++k) {
                float complex t = work[i + k + step] * wk;
                float complex u = work[i + k];
                work[i + k]        = u + t;
                work[i + k + step] = u - t;
                wk *= w_inc;
            }
#else
            /* Original twiddle loads for maximum numerical stability */
            unsigned k = 0;
            for (; k + 1 < step; k += 2) {
#if defined(LORA_LITE_EMB_THROUGHPUT) && (defined(__arm__) || defined(__aarch64__))
                __builtin_prefetch(&work[i + k + 8], 0, 1);
                __builtin_prefetch(&work[i + k + step + 8], 0, 1);
#endif
                float complex w0 = tw[(k + 0) * tw_step];
                float complex w1 = tw[(k + 1) * tw_step];
                float complex a0 = work[i + k + 0];
                float complex a1 = work[i + k + step + 0];
                float complex b0 = work[i + k + 1];
                float complex b1 = work[i + k + step + 1];
#if defined(LORA_LITE_NEON) && defined(__ARM_NEON)
                float32x2_t a1v = vld1_f32((const float*)&a1);
                float32x2_t w0v = vld1_f32((const float*)&w0);
                float32x2_t b1v = vld1_f32((const float*)&b1);
                float32x2_t w1v = vld1_f32((const float*)&w1);
                float32x2_t t0v = { a1v[0]*w0v[0] - a1v[1]*w0v[1], a1v[0]*w0v[1] + a1v[1]*w0v[0] };
                float32x2_t t1v = { b1v[0]*w1v[0] - b1v[1]*w1v[1], b1v[0]*w1v[1] + b1v[1]*w1v[0] };
                float complex t0, t1;
                vst1_f32((float*)&t0, t0v);
                vst1_f32((float*)&t1, t1v);
#else
                float complex t0 = a1 * w0;
                float complex t1 = b1 * w1;
#endif
                work[i + k + 0]        = a0 + t0;
                work[i + k + step + 0] = a0 - t0;
                work[i + k + 1]        = b0 + t1;
                work[i + k + step + 1] = b0 - t1;
            }
            for (; k < step; ++k) {
                float complex t = work[i + k + step] * tw[k * tw_step];
                float complex u = work[i + k];
                work[i + k]        = u + t;
                work[i + k + step] = u - t;
            }
#endif
        }
        step = jump;
    }
    /* Allow aliasing: if out == work, avoid final copy. */
    if (out != work) memcpy(out, work, n * sizeof(float complex));
}

void lora_fft_exec_inv(const lora_fft_ctx_t *ctx,
                       const float complex *restrict in,
                       float complex *restrict out) {
    unsigned n = ctx->n;
    float complex *restrict work = ctx->work;
    const float complex *restrict tw = ctx->tw;
#if defined(__GNUC__)
    work = (float complex *restrict)__builtin_assume_aligned(work, 32);
    tw   = (const float complex *restrict)__builtin_assume_aligned(tw, 32);
    in   = (const float complex *restrict)__builtin_assume_aligned(in, 32);
    out  = (float complex *restrict)__builtin_assume_aligned(out, 32);
#endif
    if (ctx->use_liquid && ctx->liquid_plan_inv) {
#ifdef LORA_LITE_USE_LIQUID_FFT
        memcpy(work, in, n * sizeof(float complex));
        fft_execute((fftplan)ctx->liquid_plan_inv);
        /* Liquid plans are unscaled; apply 1/n here. */
        const float s = 1.0f / (float)n;
        for (unsigned i = 0; i < n; ++i) out[i] = work[i] * s;
        return;
#endif
    }

#if defined(LORA_LITE_FFT_BITREV_COPY)
    unsigned log2n = 0; while ((1u << log2n) < n) ++log2n;
    for (unsigned i = 0; i < n; ++i) {
        unsigned r = bitrev_index(i, log2n);
        work[r] = in[i];
    }
#else
    memcpy(work, in, n * sizeof(float complex));
    bit_reverse(work, n);
#endif

    unsigned step = 1;
#if defined(LORA_LITE_FFT_RADIX4)
    /* Radix-4 inverse passes: use conjugated twiddles and mirrored rotations. */
    while ((step << 2) <= n) {
        unsigned jump = step << 2;
        unsigned tw_step = n / jump;
        for (unsigned i = 0; i < n; i += jump) {
            for (unsigned k = 0; k < step; ++k) {
                float complex w1 = conjf(tw[(k * 1u) * tw_step]);
                float complex w2 = conjf(tw[(k * 2u) * tw_step]);
                float complex w3 = conjf(tw[(k * 3u) * tw_step]);
                float complex a0 = work[i + k + 0*step];
                float complex a1 = work[i + k + 1*step] * w1;
                float complex a2 = work[i + k + 2*step] * w2;
                float complex a3 = work[i + k + 3*step] * w3;
                float complex t0 = a0 + a2;
                float complex t1 = a0 - a2;
                float complex t2 = a1 + a3;
                float complex t3 = (a1 - a3) * (I);
                work[i + k + 0*step] = t0 + t2;
                work[i + k + 1*step] = t1 + t3;
                work[i + k + 2*step] = t0 - t2;
                work[i + k + 3*step] = t1 - t3;
            }
        }
        step = jump;
    }
#endif
    while (step < n) {
        unsigned jump = step << 1;
        unsigned tw_step = n / jump;
        for (unsigned i = 0; i < n; i += jump) {
#if defined(LORA_LITE_TWIDDLE_RECURRENCE)
            float complex w_inc = conjf(tw[tw_step]);
            float complex w_inc2 = w_inc * w_inc;
            float complex wk = 1.0f;
            unsigned k = 0;
            for (; k + 1 < step; k += 2) {
                float complex w0 = wk;
                float complex w1 = wk * w_inc;
                float complex a0 = work[i + k + 0];
                float complex a1 = work[i + k + step + 0];
                float complex b0 = work[i + k + 1];
                float complex b1 = work[i + k + step + 1];
                float complex t0 = a1 * w0;
                float complex t1 = b1 * w1;
                work[i + k + 0]        = a0 + t0;
                work[i + k + step + 0] = a0 - t0;
                work[i + k + 1]        = b0 + t1;
                work[i + k + step + 1] = b0 - t1;
                wk *= w_inc2;
            }
            for (; k < step; ++k) {
                float complex t = work[i + k + step] * wk;
                float complex u = work[i + k];
                work[i + k]        = u + t;
                work[i + k + step] = u - t;
                wk *= w_inc;
            }
#else
            unsigned k = 0;
            for (; k + 1 < step; k += 2) {
#if defined(LORA_LITE_EMB_THROUGHPUT) && (defined(__arm__) || defined(__aarch64__))
                __builtin_prefetch(&work[i + k + 8], 0, 1);
                __builtin_prefetch(&work[i + k + step + 8], 0, 1);
#endif
                float complex w0 = conjf(tw[(k + 0) * tw_step]);
                float complex w1 = conjf(tw[(k + 1) * tw_step]);
                float complex a0 = work[i + k + 0];
                float complex a1 = work[i + k + step + 0];
                float complex b0 = work[i + k + 1];
                float complex b1 = work[i + k + step + 1];
#if defined(LORA_LITE_NEON) && defined(__ARM_NEON)
                float32x2_t a1v = vld1_f32((const float*)&a1);
                float32x2_t w0v = vld1_f32((const float*)&w0);
                float32x2_t b1v = vld1_f32((const float*)&b1);
                float32x2_t w1v = vld1_f32((const float*)&w1);
                float32x2_t t0v = { a1v[0]*w0v[0] - a1v[1]*w0v[1], a1v[0]*w0v[1] + a1v[1]*w0v[0] };
                float32x2_t t1v = { b1v[0]*w1v[0] - b1v[1]*w1v[1], b1v[0]*w1v[1] + b1v[1]*w1v[0] };
                float complex t0, t1;
                vst1_f32((float*)&t0, t0v);
                vst1_f32((float*)&t1, t1v);
#else
                float complex t0 = a1 * w0;
                float complex t1 = b1 * w1;
#endif
                work[i + k + 0]        = a0 + t0;
                work[i + k + step + 0] = a0 - t0;
                work[i + k + 1]        = b0 + t1;
                work[i + k + step + 1] = b0 - t1;
            }
            for (; k < step; ++k) {
                float complex t = work[i + k + step] * conjf(tw[k * tw_step]);
                float complex u = work[i + k];
                work[i + k]        = u + t;
                work[i + k + step] = u - t;
            }
#endif
        }
        step = jump;
    }
    /* Scale by 1/n for inverse */
    const float s = 1.0f / (float)n;
    if (out == work) {
        for (unsigned i = 0; i < n; ++i) out[i] = work[i] * s;
    } else {
        for (unsigned i = 0; i < n; ++i) out[i] = work[i] * s;
    }
}
