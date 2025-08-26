#ifndef LORA_MOD_H
#define LORA_MOD_H

#include "lora_config.h"
#include "lora_fixed.h"
#include <complex.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Generate LoRa baseband chips for given symbols.
 *
 * The caller must supply a chips buffer with at least nsym*sps elements,
 * where sps = (1u<<sf)*(samp_rate/bw) and sps <= LORA_MAX_SPS.
 */
void lora_modulate(const uint32_t *restrict symbols,
                   float complex *restrict chips, uint8_t sf,
                   uint32_t samp_rate, uint32_t bw, size_t nsym);

#endif /* LORA_MOD_H */
