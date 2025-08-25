#include "lora_chain.h"
#include "lora_fft_demod.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int lora_rx_chain(const float complex *chips, size_t nchips,
                  uint8_t **payload_out, size_t *payload_len_out)
{
    const uint8_t sf = 8;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;
    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t nsym = nchips / sps;

    uint32_t *symbols = (uint32_t *)malloc(nsym * sizeof(uint32_t));
    if (!symbols)
        return -1;
    lora_fft_demod(chips, symbols, sf, samp_rate, bw, 0.0f, nsym);

    uint8_t *whitened = (uint8_t *)malloc(nsym);
    if (!whitened) {
        free(symbols);
        return -1;
    }
    for (size_t i = 0; i < nsym; ++i)
        whitened[i] = (uint8_t)(symbols[i] & 0xFF);
    free(symbols);

    uint8_t *payload_crc = (uint8_t *)malloc(nsym);
    if (!payload_crc) {
        free(whitened);
        return -1;
    }
    lora_dewhiten(whitened, payload_crc, nsym);
    free(whitened);

    if (nsym < 2) {
        free(payload_crc);
        return -1;
    }
    size_t payload_len = nsym - 2;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t *tmp = (uint8_t *)malloc(nsym);
    if (!tmp) {
        free(payload_crc);
        return -1;
    }
    memcpy(tmp, payload_crc, nsym);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym, crc_n);
    free(tmp);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2) {
        free(payload_crc);
        return -1;
    }

    uint8_t *payload = (uint8_t *)malloc(payload_len);
    if (!payload) {
        free(payload_crc);
        return -1;
    }
    memcpy(payload, payload_crc, payload_len);
    free(payload_crc);

    *payload_out = payload;
    *payload_len_out = payload_len;
    return 0;
}

int lora_rx_run(const char *bin_in, const char *file_out)
{
    FILE *fi = fopen(bin_in, "rb");
    if (!fi)
        return -1;
    fseek(fi, 0, SEEK_END);
    long fsz = ftell(fi);
    rewind(fi);
    if (fsz < 0) {
        fclose(fi);
        return -1;
    }
    size_t nchips = (size_t)fsz / sizeof(float complex);
    float complex *chips = (float complex *)malloc(nchips * sizeof(float complex));
    if (!chips) {
        fclose(fi);
        return -1;
    }
    size_t rd = fread(chips, sizeof(float complex), nchips, fi);
    fclose(fi);
    if (rd != nchips) {
        free(chips);
        return -1;
    }

    uint8_t *payload;
    size_t payload_len;
    if (lora_rx_chain(chips, nchips, &payload, &payload_len) != 0) {
        free(chips);
        return -1;
    }
    free(chips);

    FILE *fo = fopen(file_out, "wb");
    if (!fo) {
        free(payload);
        return -1;
    }
    fwrite(payload, 1, payload_len, fo);
    fclose(fo);
    free(payload);
    return 0;
}

