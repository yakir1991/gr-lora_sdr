#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif
#include "lora_chain.h"
#include "lora_fft_demod.h"
#include "lora_fixed.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include "lora_config.h"

static uint32_t lcg_rand(uint32_t *state) {
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static float randf(uint32_t *state) {
    return (lcg_rand(state) >> 8) / (float)(1u << 24);
}

static float randn(uint32_t *state) {
    float u1 = randf(state);
    float u2 = randf(state);
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
}

int main(void) {
    const float snr_db[] = {0, 5, 10, 15, 20};
    const float ber_thresh[] = {1.0f, 0.5f, 0.1f, 0.01f, 0.0f};
    const size_t npts = sizeof(snr_db) / sizeof(snr_db[0]);

    FILE *csv = fopen("results/ber_snr.csv", "w");
    if (!csv) {
        perror("results/ber_snr.csv");
        char cwd[256];
        if (getcwd(cwd, sizeof cwd))
            fprintf(stderr, "cwd: %s\n", cwd);
        return EXIT_FAILURE;
    }
    fprintf(csv, "snr_db,ber\n");

    const uint8_t payload_len = 16;
    uint8_t payload[payload_len];
    for (uint8_t i = 0; i < payload_len; ++i) payload[i] = i;

    float complex *chips = malloc(sizeof(float complex) * LORA_MAX_CHIPS);
    if (!chips) {
        fprintf(stderr, "malloc failed\n");
        fclose(csv);
        return EXIT_FAILURE;
    }
    size_t nchips = 0;
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};
    int tx_ret = lora_tx_chain(payload, payload_len, chips, LORA_MAX_CHIPS, &nchips, &cfg);
    if (tx_ret) {
        fprintf(stderr,
                "Iteration 0: lora_tx_chain failed (ret=%d, nchips=%zu, out_len=0)\n",
                tx_ret, nchips);
        fclose(csv);
        free(chips);
        return EXIT_FAILURE;
    }

    int fail = 0;
    uint32_t rng = 12345u;
    uint32_t sps = (1u << cfg.sf) * (cfg.samp_rate / cfg.bw);
    size_t nsym = nchips / sps;

    for (size_t i = 0; i < npts; ++i) {
        float snr_linear = powf(10.0f, snr_db[i] / 10.0f);
        float noise_std = 1.0f / sqrtf(2.0f * snr_linear);
        float complex *noisy = malloc(sizeof(float complex) * nchips);
        if (!noisy) {
            fprintf(stderr, "malloc failed\n");
            fclose(csv);
            free(chips);
            return EXIT_FAILURE;
        }
        for (size_t n = 0; n < nchips; ++n) {
            float nr = noise_std * randn(&rng);
            float ni = noise_std * randn(&rng);
            noisy[n] = chips[n] + nr + I * ni;
        }

        // Run full RX chain for side effects / sanity
        uint8_t tmp_payload[LORA_MAX_PAYLOAD_LEN];
        size_t tmp_len = 0;
        int rx_ret = lora_rx_chain(noisy, nchips, tmp_payload, sizeof(tmp_payload), &tmp_len, &cfg);
        if (rx_ret) {
            fprintf(stderr,
                    "Iteration %zu: lora_rx_chain failed (ret=%d, nchips=%zu, out_len=%zu)\n",
                    i, rx_ret, nchips, tmp_len);
            free(noisy);
            free(chips);
            fclose(csv);
            return EXIT_FAILURE;
        }

        // Demodulate to compute BER
        uint32_t symbols[LORA_MAX_NSYM];
        lora_q15_complex *noisy_q = malloc(sizeof(lora_q15_complex) * nchips);
        if (!noisy_q) {
            fprintf(stderr, "malloc failed\n");
            free(noisy);
            free(chips);
            fclose(csv);
            return EXIT_FAILURE;
        }
        for (size_t n = 0; n < nchips; ++n)
            noisy_q[n] = lora_float_to_q15(noisy[n]);
        lora_fft_demod(noisy_q, symbols, cfg.sf, cfg.samp_rate, cfg.bw, 0.0f, nsym);
        free(noisy_q);

        uint8_t whitened[LORA_MAX_NSYM];
        for (size_t s = 0; s < nsym; ++s) whitened[s] = (uint8_t)(symbols[s] & 0xFF);

        uint8_t payload_crc[LORA_MAX_NSYM];
        lora_dewhiten(whitened, payload_crc, nsym);

        size_t bit_errors = 0;
        for (size_t b = 0; b < payload_len; ++b) {
            uint8_t diff = payload[b] ^ payload_crc[b];
            bit_errors += __builtin_popcount((unsigned)diff);
        }
        float ber = (float)bit_errors / (payload_len * 8.0f);
        fprintf(csv, "%g,%g\n", snr_db[i], ber);
        printf("SNR %g dB -> BER %g\n", snr_db[i], ber);
        if (ber > ber_thresh[i]) fail = 1;
        free(noisy);
    }

    free(chips);
    fclose(csv);

    if (fail) {
        fprintf(stderr, "BER exceeded threshold\n");
        return EXIT_FAILURE;
    }
    printf("BER vs SNR test passed\n");
    return 0;
}

