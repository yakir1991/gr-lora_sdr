#include "lora_chain.h"
#include "lora_add_crc.h"
#include "lora_whitening.h"
#include "lora_mod.h"
#include "lora_config.h"
#include "lora_io.h"
#include <string.h>

int lora_tx_chain(const uint8_t *restrict payload, size_t payload_len,
                  float complex *restrict chips, size_t chips_buf_len,
                  size_t *restrict nchips_out,
                  const lora_chain_cfg *cfg)
{
    if (!payload || !chips || !nchips_out || chips_buf_len == 0 || !cfg)
        return -1;

    const uint8_t sf = cfg->sf;
    const uint32_t bw = cfg->bw;
    const uint32_t samp_rate = cfg->samp_rate;

    if (payload_len > LORA_MAX_PAYLOAD_LEN)
        return -1;

    uint8_t buf[LORA_MAX_PAYLOAD_LEN + 2];
    memcpy(buf, payload, payload_len);
    buf[payload_len] = 0;
    buf[payload_len + 1] = 0;

    uint8_t crc_n[4];
    lora_add_crc(buf, payload_len + 2, crc_n);
    uint8_t crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    buf[payload_len] = crc1;
    buf[payload_len + 1] = crc2;

    uint8_t whitened[LORA_MAX_PAYLOAD_LEN + 2];
    lora_whiten(buf, whitened, payload_len + 2);

    uint32_t nsym = (uint32_t)(payload_len + 2);
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

int lora_tx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg)
{
    if (!in || !out || !cfg)
        return -1;

    uint8_t payload[LORA_MAX_PAYLOAD_LEN];
    size_t total = 0;
    while (total < LORA_MAX_PAYLOAD_LEN) {
        size_t n = in->read(in->ctx, payload + total, LORA_MAX_PAYLOAD_LEN - total);
        if (n == 0)
            break;
        total += n;
    }

    float complex chips[LORA_MAX_CHIPS];
    size_t nchips;
    if (lora_tx_chain(payload, total, chips, LORA_MAX_CHIPS, &nchips, cfg) != 0)
        return -1;

    size_t bytes = nchips * sizeof(float complex);
    size_t wr = out->write(out->ctx, (const uint8_t *)chips, bytes);
    return (wr == bytes) ? 0 : -1;
}

