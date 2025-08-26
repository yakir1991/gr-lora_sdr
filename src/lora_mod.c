#include "lora_mod.h"
#include "lora_config.h"
#include "lora_utils.h"
#include "lora_log.h"
#include <math.h>

#ifndef LORA_LITE_FIXED_POINT

void lora_modulate(const uint32_t *restrict symbols,
                   float complex *restrict chips, uint8_t sf,
                   uint32_t samp_rate, uint32_t bw, size_t nsym) {
  uint32_t rem = samp_rate % bw;
  if (rem) {
    LORA_LOG_WARN("fs %u not multiple of bw %u (rem %u)", samp_rate, bw, rem);
    return;
  }
  uint32_t n_bins = 1u << sf;
  uint32_t os_factor = samp_rate / bw;
  uint32_t sps = n_bins * os_factor;

  if (sps > LORA_MAX_SPS)
    return;

  for (size_t s = 0; s < nsym; ++s) {
    uint32_t sym = symbols[s] & (n_bins - 1u);
    float complex *out = chips + s * sps;
    lora_build_upchirp(out, sym, sf, os_factor);
  }
}

#else /* LORA_LITE_FIXED_POINT */

#include "lora_fixed.h"

void lora_modulate(const uint32_t *restrict symbols,
                   float complex *restrict chips, uint8_t sf,
                   uint32_t samp_rate, uint32_t bw, size_t nsym) {
  uint32_t rem = samp_rate % bw;
  if (rem) {
    LORA_LOG_WARN("fs %u not multiple of bw %u (rem %u)", samp_rate, bw, rem);
    return;
  }
  uint32_t n_bins = 1u << sf;
  uint32_t os_factor = samp_rate / bw;
  uint32_t sps = n_bins * os_factor;

  if (sps > LORA_MAX_SPS)
    return;

  const float q15_scale = 32767.0f;
  const float q15_inv = 1.0f / 32767.0f;

  for (size_t s = 0; s < nsym; ++s) {
    uint32_t sym = symbols[s] & (n_bins - 1u);
    float complex *out = chips + s * sps;
    lora_build_upchirp(out, sym, sf, os_factor);
    for (uint32_t n = 0; n < sps; ++n) {
      float re = crealf(out[n]);
      float im = cimagf(out[n]);
      int16_t qr = (int16_t)(re * q15_scale + (re >= 0 ? 0.5f : -0.5f));
      int16_t qi = (int16_t)(im * q15_scale + (im >= 0 ? 0.5f : -0.5f));
      out[n] = (float)qr * q15_inv + I * (float)qi * q15_inv;
    }
  }
}

#endif /* LORA_LITE_FIXED_POINT */
