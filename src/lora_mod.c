#include "lora_mod.h"
#include "lora_config.h"
#include "lora_utils.h"
#include "lora_log.h"
#include <math.h>
#include <string.h>

#ifndef LORA_LITE_FIXED_POINT

void lora_modulate(const uint32_t *symbols, float complex *chips, uint8_t sf,
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

  float complex tmp[LORA_MAX_SPS];

  for (size_t s = 0; s < nsym; ++s) {
    uint32_t sym = symbols[s] & (n_bins - 1u);
    lora_build_upchirp(tmp, sym, sf, os_factor);
    memcpy(&chips[s * sps], tmp, sps * sizeof(float complex));
  }
}

#else /* LORA_LITE_FIXED_POINT */

#include "lora_fixed.h"

void lora_modulate(const uint32_t *symbols, float complex *chips, uint8_t sf,
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

  float complex ftmp[LORA_MAX_SPS];
  lora_q15_complex qtmp[LORA_MAX_SPS];

  for (size_t s = 0; s < nsym; ++s) {
    uint32_t sym = symbols[s] & (n_bins - 1u);
    lora_build_upchirp(ftmp, sym, sf, os_factor);
    for (uint32_t n = 0; n < sps; ++n) {
      qtmp[n] = lora_float_to_q15(ftmp[n]);
      chips[s * sps + n] = lora_q15_to_float(qtmp[n]);
    }
  }
}

#endif /* LORA_LITE_FIXED_POINT */
