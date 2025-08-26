#include "lora_chain.h"
#include <stdio.h>
#include "lora_log.h"
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
        LORA_LOG_ERR("Usage: %s <input_file> <output_bin>", argv[0]);
        return 1;
    }
    const char *in_path = argv[1];
    const char *out_path = argv[2];

    FILE *fi = fopen(in_path, "rb");
    if (!fi) {
        LORA_LOG_ERR("fopen failed for %s", in_path);
        return 1;
    }
    lora_io_t in_io;
    lora_io_init_file(&in_io, fi);
    /* Move large buffers off the stack (avoid 8MB default stack overflow) */
    static uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t rd = 0;
    while (rd < LORA_MAX_PAYLOAD_LEN) {
        size_t n = in_io.read(in_io.ctx, payload + rd, LORA_MAX_PAYLOAD_LEN - rd);
        if (n == 0)
            break;
        rd += n;
    }
    fclose(fi);
    if (rd == 0) {
        LORA_LOG_ERR("input file is empty or unreadable: %s", in_path);
        return 2;
    }

    static float complex chips[LORA_MAX_CHIPS];
    size_t nchips;
    const lora_chain_cfg cfg = {.sf = 8, .bw = 125000, .samp_rate = 125000};
    if (lora_tx_chain(payload, rd, chips, LORA_MAX_CHIPS, &nchips, &cfg) != 0)
        return 1;

    const float snr_db = 30.0f;
    float noise_voltage = powf(10.0f, -snr_db / 20.0f);
    for (size_t i = 0; i < nchips; ++i) {
        float nre = rand_normal() * noise_voltage;
        float nim = rand_normal() * noise_voltage;
        chips[i] += nre + I * nim;
    }

    static uint8_t out_payload[LORA_MAX_PAYLOAD_LEN];
    size_t out_len;
    if (lora_rx_chain(chips, nchips, out_payload, sizeof(out_payload), &out_len, &cfg) != 0)
        return 1;

    FILE *fo = fopen(out_path, "wb");
    if (!fo) {
        LORA_LOG_ERR("fopen failed for %s", out_path);
        return 1;
    }
    lora_io_t out_io;
    lora_io_init_file(&out_io, fo);
    if (out_len == 0) {
        LORA_LOG_ERR("decoder produced empty payload, refusing to write");
        fclose(fo);
        return 3;
    }
    if (out_io.write(out_io.ctx, out_payload, out_len) != out_len) {
        fclose(fo);
        return 1;
    }
    fclose(fo);
    return 0;
}
