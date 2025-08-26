#ifndef Q15_TO_CF_H
#define Q15_TO_CF_H

#include <complex.h>
#include <stddef.h>
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"

void q15_to_cf(float complex *restrict dst,
               const lora_q15_complex *restrict src,
               size_t n);
#endif

#endif /* Q15_TO_CF_H */
