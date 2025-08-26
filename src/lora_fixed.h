#ifndef LORA_FIXED_H
#define LORA_FIXED_H

#ifdef LORA_LITE_FIXED_POINT
#include <complex.h>
#include <stdint.h>

#ifndef q15
typedef int16_t q15;
#endif

static inline __attribute__((always_inline)) q15
liquid_float_to_fixed(float x) {
  if (x > 0.999969f)
    x = 0.999969f;
  if (x < -1.0f)
    x = -1.0f;
  return (q15)(x * 32767.0f + (x >= 0 ? 0.5f : -0.5f));
}

static inline __attribute__((always_inline)) float
liquid_fixed_to_float(q15 x) {
  return (float)x / 32767.0f;
}

typedef struct {
  q15 r;
  q15 i;
} lora_q15_complex;

static inline __attribute__((always_inline)) lora_q15_complex
lora_float_to_q15(float complex x) {
  lora_q15_complex out;
  out.r = liquid_float_to_fixed(crealf(x));
  out.i = liquid_float_to_fixed(cimagf(x));
  return out;
}

static inline __attribute__((always_inline)) float complex
lora_q15_to_float(lora_q15_complex x) {
  return liquid_fixed_to_float(x.r) + I * liquid_fixed_to_float(x.i);
}

static inline __attribute__((always_inline)) lora_q15_complex
lora_q15_mul(lora_q15_complex a, lora_q15_complex b) {
  int32_t tr = (int32_t)a.r * b.r - (int32_t)a.i * b.i;
  int32_t ti = (int32_t)a.r * b.i + (int32_t)a.i * b.r;
  lora_q15_complex c;
  c.r = (q15)(tr >> 15);
  c.i = (q15)(ti >> 15);
  return c;
}
#endif /* LORA_LITE_FIXED_POINT */

#endif /* LORA_FIXED_H */
