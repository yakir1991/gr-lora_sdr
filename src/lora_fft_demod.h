#ifndef LORA_FFT_DEMOD_H
#define LORA_FFT_DEMOD_H

#include "lora_config.h"
#include "lora_fixed.h"
#include <complex.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Recover symbols from LoRa chips using an FFT-based demodulator.
 *
 * The caller must provide a symbols buffer with at least nsym elements.
 * The implementation uses internal scratch buffers sized by LORA_MAX_SPS.
 */
void lora_fft_demod(const float complex *chips, uint32_t *symbols, uint8_t sf,
                    uint32_t samp_rate, uint32_t bw, float freq_offset,
                    size_t nsym);

#endif /* LORA_FFT_DEMOD_H */
