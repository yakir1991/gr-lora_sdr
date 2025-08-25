#include "lora_chain.h"
#include "lora_fft_demod.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include "lora_config.h"
#include <string.h>
#include <stdio.h>

int lora_rx_chain(const float complex *chips, size_t nchips,
                  uint8_t *payload, size_t payload_buf_len,
                  size_t *payload_len_out)
{
    const uint8_t sf = 8;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;
    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t nsym = nchips / sps;
    if (nsym > LORA_MAX_NSYM)
        return -1;

    uint32_t symbols[LORA_MAX_NSYM];
    lora_fft_demod(chips, symbols, sf, samp_rate, bw, 0.0f, nsym);

    uint8_t whitened[LORA_MAX_NSYM];
    for (size_t i = 0; i < nsym; ++i)
        whitened[i] = (uint8_t)(symbols[i] & 0xFF);

    uint8_t payload_crc[LORA_MAX_NSYM];
    lora_dewhiten(whitened, payload_crc, nsym);

    if (nsym < 2)
        return -1;
    size_t payload_len = nsym - 2;
    if (payload_len > payload_buf_len)
        return -1;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t tmp[LORA_MAX_NSYM];
    memcpy(tmp, payload_crc, nsym);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym, crc_n);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2)
        return -1;

    memcpy(payload, payload_crc, payload_len);
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
    if (nchips > LORA_MAX_CHIPS) {
        fclose(fi);
        return -1;
    }
    float complex chips[LORA_MAX_CHIPS];
    size_t rd = fread(chips, sizeof(float complex), nchips, fi);
    fclose(fi);
    if (rd != nchips)
        return -1;

    uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t payload_len;
    if (lora_rx_chain(chips, nchips, payload, sizeof(payload), &payload_len) != 0)
        return -1;

    FILE *fo = fopen(file_out, "wb");
    if (!fo)
        return -1;
    fwrite(payload, 1, payload_len, fo);
    fclose(fo);
    return 0;
}

