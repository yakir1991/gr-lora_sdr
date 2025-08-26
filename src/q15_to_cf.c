#include "q15_to_cf.h"
#ifdef LORA_LITE_FIXED_POINT
void q15_to_cf(float complex *restrict dst,
               const lora_q15_complex *restrict src,
               size_t n) {
    for (size_t i = 0; i < n; ++i) {
        dst[i] = liquid_fixed_to_float(src[i].r) +
                 I * liquid_fixed_to_float(src[i].i);
    }
}
#endif
