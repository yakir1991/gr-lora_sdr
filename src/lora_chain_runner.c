#include "lora_chain.h"
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>

static uint32_t rng_state = 0;

static float rand_uniform(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return (rng_state + 1.0f) / 4294967296.0f;
}

static float rand_normal(void) {
    float u1 = rand_uniform();
    float u2 = rand_uniform();
    if (u1 <= 0.0f)
        u1 = 1e-9f;
    float mag = sqrtf(-2.0f * logf(u1));
    return mag * cosf(2.0f * (float)M_PI * u2);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_bin>\n", argv[0]);
        return 1;
    }
    const char *in_path = argv[1];
    const char *out_path = argv[2];

    FILE *fi = fopen(in_path, "rb");
    if (!fi) {
        perror("fopen");
        return 1;
    }
    fseek(fi, 0, SEEK_END);
    long flen = ftell(fi);
    rewind(fi);
    if (flen < 0) {
        fclose(fi);
        return 1;
    }
    uint8_t *payload = (uint8_t *)malloc((size_t)flen);
    if (!payload) {
        fclose(fi);
        return 1;
    }
    size_t rd = fread(payload, 1, (size_t)flen, fi);
    fclose(fi);
    if (rd != (size_t)flen) {
        free(payload);
        return 1;
    }

    float complex *chips;
    size_t nchips;
    if (lora_tx_chain(payload, rd, &chips, &nchips) != 0) {
        free(payload);
        return 1;
    }
    free(payload);

    const float snr_db = 30.0f;
    float noise_voltage = powf(10.0f, -snr_db / 20.0f);
    for (size_t i = 0; i < nchips; ++i) {
        float nre = rand_normal() * noise_voltage;
        float nim = rand_normal() * noise_voltage;
        chips[i] += nre + I * nim;
    }

    uint8_t *out_payload;
    size_t out_len;
    if (lora_rx_chain(chips, nchips, &out_payload, &out_len) != 0) {
        free(chips);
        return 1;
    }
    free(chips);

    FILE *fo = fopen(out_path, "wb");
    if (!fo) {
        free(out_payload);
        perror("fopen");
        return 1;
    }
    fwrite(out_payload, 1, out_len, fo);
    fclose(fo);
    free(out_payload);
    return 0;
}
