#include "lora_whitening.h"
#include "lora_utils.h"
#if defined(__SSE2__)
#  include <emmintrin.h>
#elif defined(__ARM_NEON)
#  include <arm_neon.h>
#endif

void lora_whiten(const uint8_t *in, uint8_t *out, size_t len)
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
        /* 16-byte SIMD blocks */
#if defined(__SSE2__)
        {
            size_t n16 = chunk / 16;
            for (size_t b = 0; b < n16; ++b, k += 16) {
                __m128i va = _mm_loadu_si128((const __m128i *)(pin + k));
                __m128i vb = _mm_loadu_si128((const __m128i *)(s + k));
                __m128i vx = _mm_xor_si128(va, vb);
                _mm_storeu_si128((__m128i *)(pout + k), vx);
            }
        }
#elif defined(__ARM_NEON)
        {
            size_t n16 = chunk / 16;
            for (size_t b = 0; b < n16; ++b, k += 16) {
                uint8x16_t va = vld1q_u8(pin + k);
                uint8x16_t vb = vld1q_u8(s + k);
                uint8x16_t vx = veorq_u8(va, vb);
                vst1q_u8(pout + k, vx);
            }
        }
#endif
        /* 8-byte blocks */
        size_t n64 = (chunk - k) / 8;
        for (size_t b = 0; b < n64; ++b, k += 8) {
            uint64_t a64, b64;
            __builtin_memcpy(&a64, pin + k, 8);
            __builtin_memcpy(&b64, s + k, 8);
            a64 ^= b64;
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

void lora_dewhiten(const uint8_t *in, uint8_t *out, size_t len)
{
    /* Whitening is XOR; dewhiten is identical to whiten */
    lora_whiten(in, out, len);
}
