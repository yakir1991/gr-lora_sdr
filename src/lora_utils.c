#include "lora_utils.h"
#include <math.h>
#include <stddef.h>

static uint8_t lora_whitening_seq_buf[LORA_WHITENING_SEQ_LEN];
const uint8_t *lora_whitening_seq = lora_whitening_seq_buf;

void lora_generate_whitening_seq(uint8_t *seq) {
  /* LFSR with polynomial x^8 + x^6 + x^5 + x^4 + 1 */
  uint8_t lfsr = 0xFF;
  for (size_t i = 0; i < LORA_WHITENING_SEQ_LEN; ++i) {
    seq[i] = lfsr;
    uint8_t new_bit =
        ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 0x01u;
    lfsr = (uint8_t)((lfsr << 1) | new_bit);
  }
}

__attribute__((constructor)) static void lora_whitening_seq_init(void) {
  lora_generate_whitening_seq(lora_whitening_seq_buf);
}

void lora_build_upchirp(float complex *restrict chirp, uint32_t id, uint8_t sf,
                        uint32_t os_factor) {
  const double N = (double)(1u << sf);
  const uint32_t sps = (uint32_t)(N * os_factor);
  const int n_fold = (int)(sps - id * os_factor);
  const double inv_N_os2 = 1.0 / (N * os_factor * os_factor);
  const double id_over_N = (double)id / N;
  const double slope1 = (id_over_N - 0.5) / (double)os_factor;
  const double slope2 = (id_over_N - 1.5) / (double)os_factor;
  const double k2pi = 2.0 * M_PI;

  /* Recurrence for quadratic phase chirp:
   *   chirp[n+1] = chirp[n] * r[n]
   *   r[n+1] = r[n] * step_r
   * where r[n] = exp(j*2π( (2n+1)/(2N os^2) + slope)), step_r = exp(j*2π*(1/(N os^2)))
   * and slope switches from slope1 to slope2 at n == n_fold.
   */
  float complex curr = 1.0f + I * 0.0f; /* chirp[0] */
  double inc0 = k2pi * (0.5 * inv_N_os2 + slope1);
  float complex r = cexpf(I * (float)inc0);
  float complex step_r = cexpf(I * (float)(k2pi * inv_N_os2));
  float complex slope_step = cexpf(I * (float)(k2pi * (slope2 - slope1)));

  for (uint32_t n = 0; n < (uint32_t)sps; ++n) {
    chirp[n] = curr;
    curr *= r;
    r *= step_r;
    if ((int)n + 1 == n_fold) {
      /* Next r uses slope2 instead of slope1 */
      r *= slope_step;
    }
  }
}

void lora_build_ref_chirps(float complex *restrict upchirp,
                           float complex *restrict downchirp, uint8_t sf,
                           uint32_t os_factor) {
  uint32_t sps = (1u << sf) * os_factor;
  lora_build_upchirp(upchirp, 0, sf, os_factor);
  for (uint32_t i = 0; i < sps; ++i) {
    downchirp[i] = conjf(upchirp[i]);
  }
}
