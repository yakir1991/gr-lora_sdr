#ifndef Q15_TO_CF_H
#define Q15_TO_CF_H

#include "lora_config.h"

#ifdef LORA_LITE_FIXED_POINT
#include <complex.h>
#include <stddef.h>
#include "lora_fixed.h"

void q15_to_cf(float complex *restrict dst,
               const lora_q15_complex *restrict src,
               size_t n);
#endif /* LORA_LITE_FIXED_POINT */

#endif /* Q15_TO_CF_H */
