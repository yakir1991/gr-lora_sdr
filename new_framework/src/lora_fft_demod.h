#ifndef LORA_FFT_DEMOD_H
#define LORA_FFT_DEMOD_H

#include <stddef.h>
#include <stdint.h>
#include <complex.h>

/* Recover symbols from LoRa chips using an FFT-based demodulator. */
void lora_fft_demod(const float complex *chips, uint32_t *symbols,
                    uint8_t sf, uint32_t samp_rate, uint32_t bw,
                    float freq_offset, size_t nsym);

#endif /* LORA_FFT_DEMOD_H */
