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
  double N = (double)(1u << sf);
  uint32_t sps = (uint32_t)(N * os_factor);
  int n_fold = (int)(sps - id * os_factor);
  double inv_2Nos2 = 1.0 / (2.0 * N * os_factor * os_factor);
  double inv_os = 1.0 / os_factor;
  double id_over_N = (double)id / N;
  double slope1 = (id_over_N - 0.5) * inv_os;
  double slope2 = (id_over_N - 1.5) * inv_os;
  const double k = 2.0 * M_PI;
  for (uint32_t n = 0; n < sps; ++n) {
    double n_d = (double)n;
    double phase =
        k * (n_d * n_d * inv_2Nos2 + n_d * ((int)n < n_fold ? slope1 : slope2));
    chirp[n] = cexpf(I * (float)phase);
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
