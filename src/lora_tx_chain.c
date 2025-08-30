#include "lora_chain.h"
#include "lora_add_crc.h"
#include "lora_whitening.h"
#include "lora_mod.h"
#include "lora_config.h"
#include "lora_io.h"
#include <string.h>
#include <stdlib.h>

lora_status lora_tx_chain(const uint8_t *restrict payload, size_t payload_len,
                          float complex *restrict chips, size_t chips_buf_len,
                          size_t *restrict nchips_out,
                          const lora_chain_cfg *cfg,
                          lora_tx_workspace *ws)
{
    if (!payload || !chips || !nchips_out || chips_buf_len == 0 || !cfg || !ws)
        return LORA_ERR_INVALID_ARG;

    const uint8_t sf = cfg->sf;
    const uint32_t bw = cfg->bw;
    const uint32_t samp_rate = cfg->samp_rate;

    if (payload_len > LORA_MAX_PAYLOAD_LEN)
        return LORA_ERR_PAYLOAD_TOO_LARGE;

    uint8_t *buf = ws->buf;
    memcpy(buf, payload, payload_len);
    buf[payload_len] = 0;
    buf[payload_len + 1] = 0;

    uint8_t crc_n[4];
    lora_add_crc(buf, payload_len + 2, crc_n);
    uint8_t crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    buf[payload_len] = crc1;
    buf[payload_len + 1] = crc2;

    /* Whiten in-place to reduce workspace */
    lora_whiten(buf, buf, payload_len + 2);

    uint32_t nsym = (uint32_t)(payload_len + 2);
    if (nsym > LORA_MAX_NSYM)
        return LORA_ERR_TOO_MANY_SYMBOLS;
    uint32_t *symbols = ws->symbols;
    for (size_t i = 0; i < nsym; ++i)
        symbols[i] = buf[i];

    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t need = (size_t)nsym * sps;
    if (need > chips_buf_len)
        return LORA_ERR_BUFFER_TOO_SMALL;
    lora_modulate(symbols, chips, sf, samp_rate, bw, nsym);

    *nchips_out = need;
    return LORA_OK;
}

lora_status lora_tx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg)
{
    if (!in || !out || !cfg)
        return LORA_ERR_INVALID_ARG;

    uint8_t *payload = malloc(LORA_MAX_PAYLOAD_LEN);
    if (!payload)
        return LORA_ERR_OOM;
    size_t total = 0;
    while (total < LORA_MAX_PAYLOAD_LEN) {
        size_t n = in->read(in->ctx, payload + total, LORA_MAX_PAYLOAD_LEN - total);
        if (n == 0)
            break;
        total += n;
    }

    float complex *chips = malloc(sizeof(float complex) * LORA_MAX_CHIPS);
    if (!chips) {
        free(payload);
        return LORA_ERR_OOM;
    }
    size_t nchips;
    static lora_tx_workspace ws;
    lora_status st = lora_tx_chain(payload, total, chips, LORA_MAX_CHIPS, &nchips, cfg, &ws);
    if (st != LORA_OK) {
        free(payload);
        free(chips);
        return st;
    }

    size_t bytes = nchips * sizeof(float complex);
    size_t wr = out->write(out->ctx, (const uint8_t *)chips, bytes);
    free(payload);
    free(chips);
    return (wr == bytes) ? LORA_OK : LORA_ERR_IO;
}
