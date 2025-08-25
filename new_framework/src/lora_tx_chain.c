#include "lora_chain.h"
#include "lora_add_crc.h"
#include "lora_whitening.h"
#include "lora_mod.h"
#include "lora_data_source.h"
#include <stdlib.h>
#include <string.h>

int lora_tx_chain(const uint8_t *payload, size_t payload_len,
                  float complex **chips_out, size_t *nchips_out)
{
    const uint8_t sf = 8;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;

    size_t total_bytes = payload_len + 2;
    uint8_t *buf = (uint8_t *)malloc(total_bytes);
    if (!buf)
        return -1;
    memcpy(buf, payload, payload_len);
    buf[payload_len] = 0;
    buf[payload_len + 1] = 0;

    uint8_t crc_n[4];
    lora_add_crc(buf, total_bytes, crc_n);
    uint8_t crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    buf[payload_len] = crc1;
    buf[payload_len + 1] = crc2;

    uint8_t *whitened = (uint8_t *)malloc(total_bytes);
    if (!whitened) {
        free(buf);
        return -1;
    }
    lora_whiten(buf, whitened, total_bytes);

    uint32_t nsym = (uint32_t)total_bytes;
    uint32_t *symbols = (uint32_t *)malloc(nsym * sizeof(uint32_t));
    if (!symbols) {
        free(buf);
        free(whitened);
        return -1;
    }
    for (size_t i = 0; i < nsym; ++i)
        symbols[i] = whitened[i];

    uint32_t sps = (1u << sf) * (samp_rate / bw);
    float complex *chips = (float complex *)malloc((size_t)nsym * sps * sizeof(float complex));
    if (!chips) {
        free(buf);
        free(whitened);
        free(symbols);
        return -1;
    }
    lora_modulate(symbols, chips, sf, samp_rate, bw, nsym);

    free(buf);
    free(whitened);
    free(symbols);

    *chips_out = chips;
    *nchips_out = (size_t)nsym * sps;
    return 0;
}

int lora_tx_run(const char *file_in, const char *bin_out)
{
    lora_data_source_t *src = lora_data_source_open(file_in);
    if (!src)
        return -1;
    fseek(src->fp, 0, SEEK_END);
    long flen = ftell(src->fp);
    rewind(src->fp);
    if (flen < 0) {
        lora_data_source_close(src);
        return -1;
    }
    uint8_t *payload = (uint8_t *)malloc((size_t)flen);
    if (!payload) {
        lora_data_source_close(src);
        return -1;
    }
    size_t rd = lora_data_source_read(src, payload, (size_t)flen);
    lora_data_source_close(src);
    if (rd != (size_t)flen) {
        free(payload);
        return -1;
    }

    float complex *chips;
    size_t nchips;
    if (lora_tx_chain(payload, rd, &chips, &nchips) != 0) {
        free(payload);
        return -1;
    }
    free(payload);

    FILE *fo = fopen(bin_out, "wb");
    if (!fo) {
        free(chips);
        return -1;
    }
    fwrite(chips, sizeof(float complex), nchips, fo);
    fclose(fo);
    free(chips);
    return 0;
}

