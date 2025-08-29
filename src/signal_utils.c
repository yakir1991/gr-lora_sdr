#include "signal_utils.h"
#include <math.h>
#if defined(__ARM_NEON)
#  include <arm_neon.h>
#elif defined(__SSE2__)
#  include <emmintrin.h>
#endif

/**
 * Compute the root mean square of an array of samples.
 */
double rms(const double *data, size_t len) {
    if (!data || len == 0) {
        return 0.0;
    }
    /* Improve numerical stability with Neumaier-compensated summation. */
    double sum = 0.0;
    double c = 0.0; /* compensation for lost low-order bits */
    for (size_t i = 0; i < len; ++i) {
        double yi = data[i] * data[i] - c;
        double t = sum + yi;
        /* (|sum| >= |yi|) ? compensation = (t - sum) - yi : (t - yi) - sum */
        c = (fabs(sum) >= fabs(yi)) ? (t - sum) - yi : (t - yi) - sum;
        sum = t;
    }
    return sqrt(sum / (double)len);
}

float rmsf(const float *data, size_t len) {
    if (!data || len == 0) {
        return 0.0f;
    }
    float sum = 0.0f;

#if defined(__ARM_NEON)
    /* Vectorized sum of squares using NEON */
    size_t i = 0;
    float32x4_t vsum = vdupq_n_f32(0.0f);
    for (; i + 4 <= len; i += 4) {
        float32x4_t v = vld1q_f32(data + i);
        vsum = vmlaq_f32(vsum, v, v); /* vsum += v*v */
    }
    float32x2_t vlow = vget_low_f32(vsum);
    float32x2_t vhigh = vget_high_f32(vsum);
    float32x2_t pair = vadd_f32(vlow, vhigh);
    float sum_vec = vget_lane_f32(pair, 0) + vget_lane_f32(pair, 1);
    sum = sum_vec;
    for (; i < len; ++i) {
        sum += data[i] * data[i];
    }
    return sqrtf(sum / (float)len);
#elif defined(__SSE2__)
    /* Vectorized sum of squares using SSE2 */
    size_t i = 0;
    __m128 vsum = _mm_setzero_ps();
    for (; i + 4 <= len; i += 4) {
        __m128 v = _mm_loadu_ps(data + i);
        __m128 vv = _mm_mul_ps(v, v);
        vsum = _mm_add_ps(vsum, vv);
    }
    /* Horizontal sum without SSE3 */
    __m128 shuf = _mm_movehl_ps(vsum, vsum);
    __m128 sums = _mm_add_ps(vsum, shuf);
    shuf = _mm_shuffle_ps(sums, sums, 0x55); /* move element 1 to all lanes */
    sums = _mm_add_ss(sums, shuf);
    float sum_vec;
    _mm_store_ss(&sum_vec, sums);
    sum = sum_vec;
    for (; i < len; ++i) {
        sum += data[i] * data[i];
    }
    return sqrtf(sum / (float)len);
#else
    /* Fallback: Neumaier-compensated summation in float */
    float c = 0.0f;
    for (size_t i = 0; i < len; ++i) {
        float yi = data[i] * data[i] - c;
        float t = sum + yi;
        c = (fabsf(sum) >= fabsf(yi)) ? (t - sum) - yi : (t - yi) - sum;
        sum = t;
    }
    return sqrtf(sum / (float)len);
#endif
}
