#include "lora_chain.h"
#include "lora_add_crc.h"
#include "lora_whitening.h"
#include "lora_mod.h"
#include "lora_data_source.h"
#include "lora_config.h"
#include <string.h>

int lora_tx_chain(const uint8_t *payload, size_t payload_len,
                  float complex *chips, size_t chips_buf_len,
                  size_t *nchips_out)
{
    const uint8_t sf = 8;
    const uint32_t bw = 125000;
    const uint32_t samp_rate = 125000;

    size_t total_bytes = payload_len + 2;
    if (total_bytes > LORA_MAX_PAYLOAD_LEN)
        return -1;
    uint8_t buf[LORA_MAX_PAYLOAD_LEN];
    memcpy(buf, payload, payload_len);
    buf[payload_len] = 0;
    buf[payload_len + 1] = 0;

    uint8_t crc_n[4];
    lora_add_crc(buf, total_bytes, crc_n);
    uint8_t crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    buf[payload_len] = crc1;
    buf[payload_len + 1] = crc2;

    uint8_t whitened[LORA_MAX_PAYLOAD_LEN];
    lora_whiten(buf, whitened, total_bytes);

    uint32_t nsym = (uint32_t)total_bytes;
    if (nsym > LORA_MAX_NSYM)
        return -1;
    uint32_t symbols[LORA_MAX_NSYM];
    for (size_t i = 0; i < nsym; ++i)
        symbols[i] = whitened[i];

    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t need = (size_t)nsym * sps;
    if (need > chips_buf_len)
        return -1;
    lora_modulate(symbols, chips, sf, samp_rate, bw, nsym);

    *nchips_out = need;
    return 0;
}

int lora_tx_run(const char *file_in, const char *bin_out)
{
    lora_data_source_t src;
    if (lora_data_source_open(&src, file_in) != 0)
        return -1;
    fseek(src.fp, 0, SEEK_END);
    long flen = ftell(src.fp);
    rewind(src.fp);
    if (flen < 0) {
        lora_data_source_close(&src);
        return -1;
    }
    if ((size_t)flen > LORA_MAX_PAYLOAD_LEN) {
        lora_data_source_close(&src);
        return -1;
    }
    uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t rd = lora_data_source_read(&src, payload, (size_t)flen);
    lora_data_source_close(&src);
    if (rd != (size_t)flen)
        return -1;

    float complex chips[LORA_MAX_CHIPS];
    size_t nchips;
    if (lora_tx_chain(payload, rd, chips, LORA_MAX_CHIPS, &nchips) != 0)
        return -1;

    FILE *fo = fopen(bin_out, "wb");
    if (!fo)
        return -1;
    fwrite(chips, sizeof(float complex), nchips, fo);
    fclose(fo);
    return 0;
}

