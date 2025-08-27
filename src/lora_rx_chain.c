#include "lora_chain.h"
#include "lora_config.h"
#include "lora_io.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include <string.h>
#include <stdlib.h>

#include "lora_fft_demod.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#endif

lora_status lora_rx_chain(const float complex *restrict chips, size_t nchips,
                          uint8_t *restrict payload, size_t payload_buf_len,
                          size_t *restrict payload_len_out,
                          const lora_chain_cfg *cfg,
                          lora_rx_workspace *ws)
{
    if (!chips || !payload || !payload_len_out || payload_buf_len == 0 || !cfg || !ws)
        return LORA_ERR_INVALID_ARG;

    const uint8_t sf = cfg->sf;
    const uint32_t bw = cfg->bw;
    const uint32_t samp_rate = cfg->samp_rate;
    uint32_t sps = (1u << sf) * (samp_rate / bw);
    size_t nsym = nchips / sps;
    if (nsym > LORA_MAX_NSYM)
        return LORA_ERR_TOO_MANY_SYMBOLS;

    uint32_t *symbols = ws->symbols;

#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex *qchips = ws->qchips;
    const float q15_scale = 32767.0f;
    for (size_t i = 0; i < nchips && i < LORA_MAX_CHIPS; ++i) {
        float re = crealf(chips[i]);
        if (re > 0.999969f)
            re = 0.999969f;
        if (re < -1.0f)
            re = -1.0f;
        float im = cimagf(chips[i]);
        if (im > 0.999969f)
            im = 0.999969f;
        if (im < -1.0f)
            im = -1.0f;
        qchips[i].r = (int16_t)(re * q15_scale + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * q15_scale + (im >= 0 ? 0.5f : -0.5f));
    }
    const lora_q15_complex *chip_ptr = qchips;
#else
    const float complex *chip_ptr = chips;
#endif

    lora_fft_demod_ctx_t *ctx = &ws->demod_ctx;
    if (!ws->fft_ws)
        return LORA_ERR_INVALID_ARG;
    lora_fft_demod(ctx, chip_ptr, nsym, symbols);

    uint8_t *whitened = ws->whitened;
    for (size_t i = 0; i < nsym; ++i)
        whitened[i] = (uint8_t)(symbols[i] & 0xFF);

    uint8_t *payload_crc = ws->payload_crc;
    lora_dewhiten(whitened, payload_crc, nsym);

    if (nsym < 2)
        return LORA_ERR_INVALID_ARG;
    size_t payload_len = nsym - 2;
    if (payload_len > payload_buf_len)
        return LORA_ERR_BUFFER_TOO_SMALL;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t *tmp = ws->tmp;
    memcpy(tmp, payload_crc, nsym);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym, crc_n);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2)
        return LORA_ERR_CRC_MISMATCH;

    memcpy(payload, payload_crc, payload_len);
    *payload_len_out = payload_len;
    return LORA_OK;
}

lora_status lora_rx_chain_init(lora_rx_workspace *ws, uint8_t sf,
                               uint32_t samp_rate, uint32_t bw) {
    if (!ws)
        return LORA_ERR_INVALID_ARG;
    size_t ws_bytes = lora_fft_workspace_bytes(sf, samp_rate, bw);
    if (ws_bytes == 0)
        return LORA_ERR_UNSUPPORTED;
    ws->fft_ws = aligned_alloc(32, ws_bytes);
    if (!ws->fft_ws)
        return LORA_ERR_OOM;
    if (lora_fft_demod_init(&ws->demod_ctx, sf, samp_rate, bw, ws->fft_ws,
                             ws_bytes) != 0) {
        free(ws->fft_ws);
        ws->fft_ws = NULL;
        return LORA_ERR_IO;
    }
    ws->demod_ctx.cfo = 0.0f;
    ws->demod_ctx.cfo_phase = 0.0;
    return LORA_OK;
}

void lora_rx_chain_free(lora_rx_workspace *ws) {
    if (!ws || !ws->fft_ws)
        return;
    lora_fft_demod_free(&ws->demod_ctx);
    free(ws->fft_ws);
    ws->fft_ws = NULL;
}

lora_status lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg)
{
    if (!in || !out || !cfg)
        return LORA_ERR_INVALID_ARG;

    float complex *chips = malloc(sizeof(float complex) * LORA_MAX_CHIPS);
    if (!chips)
        return LORA_ERR_OOM;
    size_t total = 0;
    size_t max_bytes = LORA_MAX_CHIPS * sizeof(float complex);
    uint8_t *chip_bytes = (uint8_t *)chips;
    while (total < max_bytes) {
        size_t n = in->read(in->ctx, chip_bytes + total, max_bytes - total);
        if (n == 0)
            break;
        total += n;
    }
    size_t nchips = total / sizeof(float complex);

    uint8_t *payload = malloc(LORA_MAX_PAYLOAD_LEN);
    if (!payload) {
        free(chips);
        return LORA_ERR_OOM;
    }
    size_t payload_len;
    static lora_rx_workspace ws;
    lora_status st = lora_rx_chain(chips, nchips, payload, LORA_MAX_PAYLOAD_LEN, &payload_len, cfg, &ws);
    if (st != LORA_OK) {
        free(chips);
        free(payload);
        return st;
    }

    size_t wr = out->write(out->ctx, payload, payload_len);
    free(chips);
    free(payload);
    return (wr == payload_len) ? LORA_OK : LORA_ERR_IO;
}

