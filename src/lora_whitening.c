#include "lora_whitening.h"
#include "lora_utils.h"
#if defined(__AVX2__)
#  include <immintrin.h>
#elif defined(__SSE2__)
#  include <emmintrin.h>
#elif defined(__ARM_NEON)
#  include <arm_neon.h>
#endif

void lora_whiten(const uint8_t *restrict in, uint8_t *restrict out, size_t len)
{
    size_t i = 0;
    size_t seq_idx = 0;
    while (i < len) {
        size_t chunk = LORA_WHITENING_SEQ_LEN - seq_idx;
        if (chunk > len - i) chunk = len - i;
        const uint8_t *s = lora_whitening_seq + seq_idx;
        const uint8_t *pin = in + i;
        uint8_t *pout = out + i;
        /* SIMD: process 16B blocks when available, else 8B, then tail */
        size_t k = 0;
        /* SIMD blocks */
#if defined(__AVX2__)
        {
            /* First, 32-byte AVX2 blocks with unroll x2 (64 bytes per iter) */
            size_t n32 = chunk / 32;
            size_t b32 = 0;
            for (; b32 + 1 < n32; b32 += 2, k += 64) {
                __m256i va0 = _mm256_loadu_si256((const __m256i *)(pin + k));
                __m256i vb0 = _mm256_loadu_si256((const __m256i *)(s + k));
                __m256i vx0 = _mm256_xor_si256(va0, vb0);
                _mm256_storeu_si256((__m256i *)(pout + k), vx0);

                __m256i va1 = _mm256_loadu_si256((const __m256i *)(pin + k + 32));
                __m256i vb1 = _mm256_loadu_si256((const __m256i *)(s + k + 32));
                __m256i vx1 = _mm256_xor_si256(va1, vb1);
                _mm256_storeu_si256((__m256i *)(pout + k + 32), vx1);
            }
            for (; b32 < n32; ++b32, k += 32) {
                __m256i va = _mm256_loadu_si256((const __m256i *)(pin + k));
                __m256i vb = _mm256_loadu_si256((const __m256i *)(s + k));
                __m256i vx = _mm256_xor_si256(va, vb);
                _mm256_storeu_si256((__m256i *)(pout + k), vx);
            }
        }
        /* Fall-through to handle any 16-byte residue via SSE2 (if available) */
#elif defined(__SSE2__)
        {
            size_t n16 = chunk / 16;
            size_t b = 0;
            /* Unroll by 2 (32 bytes) for throughput */
            for (; b + 1 < n16; b += 2, k += 32) {
                __m128i va0 = _mm_loadu_si128((const __m128i *)(pin + k));
                __m128i vb0 = _mm_loadu_si128((const __m128i *)(s + k));
                __m128i vx0 = _mm_xor_si128(va0, vb0);
                _mm_storeu_si128((__m128i *)(pout + k), vx0);

                __m128i va1 = _mm_loadu_si128((const __m128i *)(pin + k + 16));
                __m128i vb1 = _mm_loadu_si128((const __m128i *)(s + k + 16));
                __m128i vx1 = _mm_xor_si128(va1, vb1);
                _mm_storeu_si128((__m128i *)(pout + k + 16), vx1);
            }
            for (; b < n16; ++b, k += 16) {
                __m128i va = _mm_loadu_si128((const __m128i *)(pin + k));
                __m128i vb = _mm_loadu_si128((const __m128i *)(s + k));
                __m128i vx = _mm_xor_si128(va, vb);
                _mm_storeu_si128((__m128i *)(pout + k), vx);
            }
        }
#elif defined(__ARM_NEON)
        {
            size_t n16 = chunk / 16;
            size_t b = 0;
            for (; b + 1 < n16; b += 2, k += 32) {
                uint8x16_t va0 = vld1q_u8(pin + k);
                uint8x16_t vb0 = vld1q_u8(s + k);
                uint8x16_t vx0 = veorq_u8(va0, vb0);
                vst1q_u8(pout + k, vx0);

                uint8x16_t va1 = vld1q_u8(pin + k + 16);
                uint8x16_t vb1 = vld1q_u8(s + k + 16);
                uint8x16_t vx1 = veorq_u8(va1, vb1);
                vst1q_u8(pout + k + 16, vx1);
            }
            for (; b < n16; ++b, k += 16) {
                uint8x16_t va = vld1q_u8(pin + k);
                uint8x16_t vb = vld1q_u8(s + k);
                uint8x16_t vx = veorq_u8(va, vb);
                vst1q_u8(pout + k, vx);
            }
        }
#endif
        /* 8-byte blocks */
        size_t n64 = (chunk - k) / 8;
        size_t b64 = 0;
        for (; b64 + 1 < n64; b64 += 2, k += 16) {
            uint64_t a0, b0, a1, b1;
            __builtin_memcpy(&a0, pin + k, 8);
            __builtin_memcpy(&b0, s + k, 8);
            __builtin_memcpy(&a1, pin + k + 8, 8);
            __builtin_memcpy(&b1, s + k + 8, 8);
            a0 ^= b0; a1 ^= b1;
            __builtin_memcpy(pout + k, &a0, 8);
            __builtin_memcpy(pout + k + 8, &a1, 8);
        }
        for (; b64 < n64; ++b64, k += 8) {
            uint64_t a64, b64v;
            __builtin_memcpy(&a64, pin + k, 8);
            __builtin_memcpy(&b64v, s + k, 8);
            a64 ^= b64v;
            __builtin_memcpy(pout + k, &a64, 8);
        }
        for (; k < chunk; ++k) {
            pout[k] = (uint8_t)(pin[k] ^ s[k]);
        }
        i += chunk;
        seq_idx += chunk;
        if (seq_idx == LORA_WHITENING_SEQ_LEN) seq_idx = 0;
    }
}

void lora_dewhiten(const uint8_t *restrict in, uint8_t *restrict out, size_t len)
{
    /* Whitening is XOR; dewhiten is identical to whiten */
    lora_whiten(in, out, len);
}
