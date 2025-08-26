#include "q15_to_cf.h"

#ifdef LORA_LITE_FIXED_POINT
void q15_to_cf(float complex *restrict dst,
               const lora_q15_complex *restrict src,
               size_t n) {
    const float scale = 1.0f / 32767.0f;
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i].r * scale + I * src[i].i * scale;
    }
}
#endif /* LORA_LITE_FIXED_POINT */
