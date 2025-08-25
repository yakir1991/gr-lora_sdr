#include "lora_fft_demod.h"
#include "lora_utils.h"
#include "lora_config.h"
#include <math.h>
#include <string.h>
#include <stdalign.h>
#include <kiss_fft.h>

void lora_fft_demod(const float complex *chips, uint32_t *symbols,
                    uint8_t sf, uint32_t samp_rate, uint32_t bw,
                    float freq_offset, size_t nsym)
{
    uint32_t n_bins = 1u << sf;
    uint32_t os_factor = samp_rate / bw;
    uint32_t sps = n_bins * os_factor;
    if (sps > LORA_MAX_SPS)
        return;

    float complex upchirp[LORA_MAX_SPS];
    float complex downchirp[LORA_MAX_SPS];
    lora_build_ref_chirps(upchirp, downchirp, sf, os_factor);

    kiss_fft_cpx cx_in[LORA_MAX_SPS];
    kiss_fft_cpx cx_out[LORA_MAX_SPS];

    size_t cfg_sz = 0;
    kiss_fft_alloc(sps, 0, NULL, &cfg_sz);
    if (cfg_sz > LORA_KISSFFT_CFG_MAX)
        return;
    alignas(16) unsigned char cfg_buf[LORA_KISSFFT_CFG_MAX];
    kiss_fft_cfg cfg = kiss_fft_alloc(sps, 0, cfg_buf, &cfg_sz);
    if (!cfg)
        return;

    for (size_t s = 0; s < nsym; ++s) {
        const float complex *symchips = &chips[s * sps];
        for (uint32_t n = 0; n < sps; ++n) {
            float complex c = symchips[n] * downchirp[n];
            if (freq_offset != 0.0f) {
                double phase = -2.0 * M_PI * freq_offset * (double)n / (double)samp_rate;
                c *= cexpf(I * (float)phase);
            }
            cx_in[n].r = crealf(c);
            cx_in[n].i = cimagf(c);
        }
        kiss_fft(cfg, cx_in, cx_out);

        float max_mag = 0.0f;
        uint32_t max_idx = 0;
        for (uint32_t i = 0; i < sps; ++i) {
            float mag = cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i;
            if (mag > max_mag) {
                max_mag = mag;
                max_idx = i;
            }
        }
        symbols[s] = (max_idx / os_factor) & (n_bins - 1u);
    }
}
