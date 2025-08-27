#include "q15_to_cf.h"

#ifdef LORA_LITE_FIXED_POINT
#if defined(__ARM_NEON)
#  include <arm_neon.h>
#endif
void q15_to_cf(float complex *restrict dst,
               const lora_q15_complex *restrict src,
               size_t n) {
    const float scale = 1.0f / 32767.0f;
#if defined(__ARM_NEON)
    /* Convert 4 complex samples (8 int16) per iteration */
    size_t i = 0;
    float32x4_t vscale = vdupq_n_f32(scale);
    for (; i + 4 <= n; i += 4) {
        int16x8_t v = vld1q_s16((const int16_t *)&src[i]); /* [r0 i0 r1 i1 r2 i2 r3 i3] */
        int16x8x2_t deint = vuzpq_s16(v, v);               /* deinterleave even/odd */
        int16x8_t re_i16 = deint.val[0];                   /* r0 r1 r2 r3 ... */
        int16x8_t im_i16 = deint.val[1];                   /* i0 i1 i2 i3 ... */
        int32x4_t re_lo = vmovl_s16(vget_low_s16(re_i16));
        int32x4_t im_lo = vmovl_s16(vget_low_s16(im_i16));
        float32x4_t re_f = vmulq_f32(vcvtq_f32_s32(re_lo), vscale);
        float32x4_t im_f = vmulq_f32(vcvtq_f32_s32(im_lo), vscale);
        float32x4x2_t out;
        out.val[0] = re_f;
        out.val[1] = im_f;
        vst2q_f32((float *)&dst[i], out); /* interleaved re,im pairs */
    }
    for (; i < n; ++i) {
        dst[i] = src[i].r * scale + I * src[i].i * scale;
    }
#else
    for (size_t i = 0; i < n; ++i) {
        dst[i] = src[i].r * scale + I * src[i].i * scale;
    }
#endif
}
#endif /* LORA_LITE_FIXED_POINT */
