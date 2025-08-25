#ifndef LORA_MOD_H
#define LORA_MOD_H

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* Generate LoRa baseband chips for given symbols. */
void lora_modulate(const uint32_t *symbols, float complex *chips,
                   uint8_t sf, uint32_t samp_rate, uint32_t bw,
                   size_t nsym);

#endif /* LORA_MOD_H */
