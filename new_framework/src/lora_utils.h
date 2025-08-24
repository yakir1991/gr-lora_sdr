#ifndef LORA_UTILS_H
#define LORA_UTILS_H

#include <stdint.h>
#include <complex.h>

#define MIN_SF 5
#define MAX_SF 12
#define LDRO_MAX_DURATION_MS 16

#define LORA_WHITENING_SEQ_LEN 255
extern const uint8_t lora_whitening_seq[LORA_WHITENING_SEQ_LEN];

void lora_build_upchirp(float complex *chirp, uint32_t id,
                        uint8_t sf, uint32_t os_factor);
void lora_build_ref_chirps(float complex *upchirp, float complex *downchirp,
                           uint8_t sf, uint32_t os_factor);

#endif /* LORA_UTILS_H */
