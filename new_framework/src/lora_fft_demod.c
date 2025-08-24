#include "lora_fft_demod.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <kiss_fft.h>

static void build_upchirp(float complex *chirp, uint32_t id,
                          uint8_t sf, uint32_t os_factor)
{
    double N = (double)(1u << sf);
    uint32_t sps = (uint32_t)(N * os_factor);
    int n_fold = (int)(sps - id * os_factor);
    for (uint32_t n = 0; n < sps; ++n) {
        double phase;
        if ((int)n < n_fold)
            phase = 2.0 * M_PI * ((double)n * (double)n / (2.0 * N * os_factor * os_factor) +
                                  ((double)id / N - 0.5) * (double)n / os_factor);
        else
            phase = 2.0 * M_PI * ((double)n * (double)n / (2.0 * N * os_factor * os_factor) +
                                  ((double)id / N - 1.5) * (double)n / os_factor);
        chirp[n] = cexpf(I * (float)phase);
    }
}

static void build_ref_chirps(float complex *upchirp, float complex *downchirp,
                             uint8_t sf, uint32_t os_factor)
{
    uint32_t sps = (1u << sf) * os_factor;
    build_upchirp(upchirp, 0, sf, os_factor);
    for (uint32_t i = 0; i < sps; ++i) {
        downchirp[i] = conjf(upchirp[i]);
    }
}

void lora_fft_demod(const float complex *chips, uint32_t *symbols,
                    uint8_t sf, uint32_t samp_rate, uint32_t bw,
                    float freq_offset, size_t nsym)
{
    uint32_t n_bins = 1u << sf;
    uint32_t os_factor = samp_rate / bw;
    uint32_t sps = n_bins * os_factor;

    float complex *upchirp = (float complex *)malloc(sps * sizeof(float complex));
    float complex *downchirp = (float complex *)malloc(sps * sizeof(float complex));
    if (!upchirp || !downchirp) {
        free(upchirp);
        free(downchirp);
        return;
    }
    build_ref_chirps(upchirp, downchirp, sf, os_factor);
    free(upchirp);

    kiss_fft_cfg cfg = kiss_fft_alloc(sps, 0, NULL, NULL);
    kiss_fft_cpx *cx_in = (kiss_fft_cpx *)malloc(sps * sizeof(kiss_fft_cpx));
    kiss_fft_cpx *cx_out = (kiss_fft_cpx *)malloc(sps * sizeof(kiss_fft_cpx));
    if (!cfg || !cx_in || !cx_out) {
        free(cfg);
        free(cx_in);
        free(cx_out);
        free(downchirp);
        return;
    }

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

    free(cfg);
    free(cx_in);
    free(cx_out);
    free(downchirp);
}
