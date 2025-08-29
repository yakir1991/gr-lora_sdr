#include "lora_chain.h"
#include "lora_config.h"
#include "lora_io.h"
#include "lora_whitening.h"
#include "lora_add_crc.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "lora_fft_demod.h"
#include "lora_frame_sync.h"
#include "lora_log.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif
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
    size_t nlim = nchips < LORA_MAX_CHIPS ? nchips : LORA_MAX_CHIPS;

    /* Convert float complex [-1,1) to Q15 interleaved pairs. NEON path when available. */
    const float *fin = (const float *)chips; /* interleaved re,im */
#if defined(__ARM_NEON)
    const float32x4_t vmax = vdupq_n_f32(0.999969f);
    const float32x4_t vmin = vdupq_n_f32(-1.0f);
    const float32x4_t vscale = vdupq_n_f32(32767.0f);
    size_t i = 0;
    for (; i + 4 <= nlim; i += 4) {
        /* load 4 complex = 8 floats */
        float32x4x2_t deint = vld2q_f32(fin + i * 2);
        float32x4_t re = vmaxq_f32(vminq_f32(deint.val[0], vmax), vmin);
        float32x4_t im = vmaxq_f32(vminq_f32(deint.val[1], vmax), vmin);
        int32x4_t re_i32 = vcvtq_s32_f32(vmulq_f32(re, vscale));
        int32x4_t im_i32 = vcvtq_s32_f32(vmulq_f32(im, vscale));
        int16x4_t re_i16 = vqmovn_s32(re_i32);
        int16x4_t im_i16 = vqmovn_s32(im_i32);
        int16x4x2_t inter;
        inter.val[0] = re_i16;
        inter.val[1] = im_i16;
        vst2_s16((int16_t *)&qchips[i], inter);
    }
    for (; i < nlim; ++i) {
        float re = fin[2*i+0];
        float im = fin[2*i+1];
        if (re > 0.999969f) re = 0.999969f; if (re < -1.0f) re = -1.0f;
        if (im > 0.999969f) im = 0.999969f; if (im < -1.0f) im = -1.0f;
        qchips[i].r = (int16_t)(re * 32767.0f + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * 32767.0f + (im >= 0 ? 0.5f : -0.5f));
    }
#else
    const float q15_scale = 32767.0f;
    for (size_t i = 0; i < nlim; ++i) {
        float re = fin[2*i+0];
        if (re > 0.999969f) re = 0.999969f; if (re < -1.0f) re = -1.0f;
        float im = fin[2*i+1];
        if (im > 0.999969f) im = 0.999969f; if (im < -1.0f) im = -1.0f;
        qchips[i].r = (int16_t)(re * q15_scale + (re >= 0 ? 0.5f : -0.5f));
        qchips[i].i = (int16_t)(im * q15_scale + (im >= 0 ? 0.5f : -0.5f));
    }
#endif
    const lora_q15_complex *chip_ptr = qchips;
#else
    const float complex *chip_ptr = chips;
#endif

    size_t ws_bytes = lora_fft_workspace_bytes(sf, samp_rate, bw);
    if (ws_bytes == 0)
        return LORA_ERR_UNSUPPORTED;
    /* Reuse persistent FFT demod workspace/context in ws. */
    if (!ws->fft_ws || ws->fft_ws_size < ws_bytes) {
#ifdef LORA_LITE_NO_MALLOC
        return LORA_ERR_BUFFER_TOO_SMALL;
#else
        /* Fallback for host builds/tests: allocate on demand */
        if (ws->fft_ws) free(ws->fft_ws);
        ws->fft_ws = aligned_alloc(32, ws_bytes);
        if (!ws->fft_ws) return LORA_ERR_OOM;
        ws->fft_ws_size = ws_bytes;
#endif
    }
    if (!ws->fft_inited || ws->fft_sf != sf || ws->fft_fs != samp_rate || ws->fft_bw != bw) {
        if (ws->fft_inited) {
            lora_fft_demod_free(&ws->fft_ctx);
            ws->fft_inited = 0;
        }
        if (lora_fft_demod_init(&ws->fft_ctx, sf, samp_rate, bw, ws->fft_ws, ws->fft_ws_size) != 0)
            return LORA_ERR_IO;
        ws->fft_ctx.cfo = 0.0f;
        ws->fft_ctx.cfo_phase = 0.0;
        ws->fft_sf = sf;
        ws->fft_fs = samp_rate;
        ws->fft_bw = bw;
        ws->fft_inited = 1;
    }
    /* First pass demod without CFO to get coarse symbols for frame sync. */
    ws->fft_ctx.cfo = 0.0f;
    ws->fft_ctx.cfo_phase = 0.0;
    lora_fft_demod(&ws->fft_ctx, chip_ptr, nsym, symbols);

    /* Estimate CFO from preamble region using dechirped phase slope. */
    const uint32_t sps_u = ws->fft_ctx.sps;
    const size_t sps_sz = (size_t)sps_u;
    size_t preamble_end = lora_frame_sync_find_preamble(symbols, nsym, 8);
    ws->sync_preamble_end = preamble_end;
    ws->sync_preamble_start = preamble_end;
    ws->sync_cfo_hz = 0.0f;
    ws->sync_preamble_match_pct = 0;
    if (preamble_end < nsym && preamble_end >= 4) {
        /* Walk back to estimate preamble start (tolerant near-zero). */
        size_t preamble_start = preamble_end;
        while (preamble_start > 0 && (symbols[preamble_start - 1] <= 1))
            --preamble_start;
        ws->sync_preamble_start = preamble_start;
        /* Compute preamble match percentage within [start,end) */
        size_t span = preamble_end > preamble_start ? (preamble_end - preamble_start) : 0;
        if (span > 0 && span < 255) {
            size_t good = 0;
            for (size_t i = preamble_start; i < preamble_end; ++i) good += (symbols[i] <= 1);
            ws->sync_preamble_match_pct = (uint8_t)((100u * good + (span/2)) / span);
        }
        /* Require at least 4 preamble symbols for a stable estimate */
        if (preamble_end > preamble_start && (preamble_end - preamble_start) >= 4) {
            size_t sym0 = preamble_start;
            size_t sym1 = preamble_end - 1;
            size_t chip_start = sym0 * sps_sz;
            size_t chip_end = sym1 * sps_sz + (sps_sz - 1);
            if (chip_end < nchips && ws->fft_ctx.downchirp) {
                float complex acc = 0.0f;
                /* Average phase increment over dechirped samples */
                for (size_t s = sym0; s <= sym1; ++s) {
                    size_t base = s * sps_sz;
                    /* Within-symbol pairs */
                    for (size_t k = 0; k + 1 < sps_u; ++k) {
                        float complex x0 = chips[base + k] * ws->fft_ctx.downchirp[k];
                        float complex x1 = chips[base + k + 1] * ws->fft_ctx.downchirp[k + 1];
                        acc += conjf(x0) * x1;
                    }
                    /* Cross-boundary pair to next symbol if still in range */
                    if (s < sym1) {
                        float complex x0 = chips[base + (sps_u - 1)] * ws->fft_ctx.downchirp[sps_u - 1];
                        float complex x1 = chips[base + sps_u] * ws->fft_ctx.downchirp[0];
                        acc += conjf(x0) * x1;
                    }
                }
                if (cabsf(acc) > 0.0f) {
                    float dphi = cargf(acc); /* radians per sample */
                    float cfo_est = (float)(dphi * (double)samp_rate / (2.0 * M_PI));
                    /* Clamp CFO estimate to a reasonable fraction of BW */
                    float max_cfo = (float)bw * 0.45f;
                    if (cfo_est > max_cfo) cfo_est = max_cfo;
                    if (cfo_est < -max_cfo) cfo_est = -max_cfo;
                    ws->fft_ctx.cfo = cfo_est;
                    ws->sync_cfo_hz = cfo_est;
                    ws->fft_ctx.cfo_phase = 0.0;
                    /* Re-run demod with CFO correction */
                    lora_fft_demod(&ws->fft_ctx, chip_ptr, nsym, symbols);
                }
            }
        }
    }

    /* Frame alignment: trim preamble+SFD if present. */
#ifndef LORA_FS_DEFAULT_PREAMBLE
#define LORA_FS_DEFAULT_PREAMBLE 8
#endif
    size_t sym_off = 0;
    {
        size_t pre_end2 = lora_frame_sync_find_preamble(symbols, nsym, LORA_FS_DEFAULT_PREAMBLE);
        if (pre_end2 < nsym) {
            size_t sfd_end = lora_frame_sync_find_sfd(symbols, nsym, pre_end2, 2, 4);
            ws->sync_sfd_end = sfd_end;
            /* Count SFD non-preamble-like symbols for metric */
            uint8_t sfd_nonzero = 0;
            if (sfd_end > pre_end2) {
                size_t sfd_len = sfd_end - pre_end2;
                for (size_t i = pre_end2; i < sfd_end; ++i)
                    sfd_nonzero += (symbols[i] > 1) ? 1 : 0;
            }
            ws->sync_sfd_nonzero = sfd_nonzero;
            sym_off = (sfd_end < nsym) ? sfd_end : 0;
        } else {
            ws->sync_sfd_end = pre_end2;
        }
    }
    ws->sync_sym_off = sym_off;

    LORA_LOG_INFO(
        "Sync metrics: pre=[%zu..%zu) match=%u%% sfd_end=%zu off=%zu CFO=%.1f Hz",
        ws->sync_preamble_start, ws->sync_preamble_end,
        ws->sync_preamble_match_pct,
        ws->sync_sfd_end, ws->sync_sym_off,
        ws->sync_cfo_hz);

    uint8_t *whitened = ws->whitened;
    size_t nsym_aligned = (sym_off < nsym) ? (nsym - sym_off) : 0;
    for (size_t i = 0; i < nsym_aligned; ++i)
        whitened[i] = (uint8_t)(symbols[sym_off + i] & 0xFF);

    uint8_t *payload_crc = ws->payload_crc;
    lora_dewhiten(whitened, payload_crc, nsym_aligned);

    if (nsym_aligned < 2)
        return LORA_ERR_INVALID_ARG;
    size_t payload_len = nsym_aligned - 2;
    if (payload_len > payload_buf_len)
        return LORA_ERR_BUFFER_TOO_SMALL;
    uint8_t crc1 = payload_crc[payload_len];
    uint8_t crc2 = payload_crc[payload_len + 1];

    uint8_t *tmp = ws->tmp;
    memcpy(tmp, payload_crc, nsym_aligned);
    tmp[payload_len] = 0;
    tmp[payload_len + 1] = 0;
    uint8_t crc_n[4];
    lora_add_crc(tmp, nsym_aligned, crc_n);
    uint8_t calc_crc1 = (uint8_t)((crc_n[1] << 4) | crc_n[0]);
    uint8_t calc_crc2 = (uint8_t)((crc_n[3] << 4) | crc_n[2]);
    if (crc1 != calc_crc1 || crc2 != calc_crc2)
        return LORA_ERR_CRC_MISMATCH;

    memcpy(payload, payload_crc, payload_len);
    *payload_len_out = payload_len;
    return LORA_OK;
}

/* Helper: bytes required for FFT demod workspace for the given config. */
size_t lora_rx_fft_workspace_bytes(const lora_chain_cfg *cfg)
{
    if (!cfg) return 0;
    return lora_fft_workspace_bytes(cfg->sf, cfg->samp_rate, cfg->bw);
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
    /* Prepare FFT demod workspace for this one-shot run (helper is allowed to malloc). */
    size_t fft_bytes = lora_rx_fft_workspace_bytes(cfg);
    ws.fft_ws = NULL; ws.fft_ws_size = 0; ws.fft_inited = 0;
    if (fft_bytes > 0) {
        ws.fft_ws = aligned_alloc(32, fft_bytes);
        if (!ws.fft_ws) { free(chips); free(payload); return LORA_ERR_OOM; }
        ws.fft_ws_size = fft_bytes;
    }
    lora_status st = lora_rx_chain(chips, nchips, payload, LORA_MAX_PAYLOAD_LEN, &payload_len, cfg, &ws);
    if (st != LORA_OK) {
        free(chips);
        free(payload);
        if (ws.fft_inited) lora_fft_demod_free(&ws.fft_ctx);
        if (ws.fft_ws) free(ws.fft_ws);
        return st;
    }

    size_t wr = out->write(out->ctx, payload, payload_len);
    free(chips);
    free(payload);
    if (ws.fft_inited) lora_fft_demod_free(&ws.fft_ctx);
    if (ws.fft_ws) free(ws.fft_ws);
    return (wr == payload_len) ? LORA_OK : LORA_ERR_IO;
}
