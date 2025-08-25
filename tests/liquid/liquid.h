#ifndef LIQUID_LIQUID_H
#define LIQUID_LIQUID_H
#include <stdint.h>

typedef int16_t q15;

static inline q15 liquid_float_to_fixed(float x) {
    if (x > 0.999969f)
        x = 0.999969f;
    if (x < -1.0f)
        x = -1.0f;
    return (q15)(x * 32768.0f);
}

static inline float liquid_fixed_to_float(q15 x) {
    return x / 32768.0f;
}

#endif
