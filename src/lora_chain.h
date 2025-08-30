#pragma once

#include <stddef.h>
#include <stdint.h>
#include <complex.h>
#include "lora_config.h"
#include "lora_io.h"
#include "lora_fft_demod.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#endif

typedef enum {
    LORA_OK = 0,             /* Success */
    LORA_ERR_INVALID_ARG,    /* NULL pointer or invalid parameter */
    LORA_ERR_PAYLOAD_TOO_LARGE, /* Payload exceeds supported size */
    LORA_ERR_TOO_MANY_SYMBOLS,  /* Input chips produce too many symbols */
    LORA_ERR_BUFFER_TOO_SMALL,  /* Output buffer too small */
    LORA_ERR_CRC_MISMATCH,   /* CRC verification failed */
    LORA_ERR_IO,             /* I/O read/write failure */
    LORA_ERR_OOM,            /* Memory allocation failed */
    LORA_ERR_UNSUPPORTED,    /* Feature not supported */
} lora_status;

typedef struct {
    uint8_t sf;
    uint32_t bw;
    uint32_t samp_rate;
} lora_chain_cfg;

typedef struct {
    uint8_t buf[LORA_MAX_PAYLOAD_LEN + 2]; /* reused for whitening in-place */
    uint32_t symbols[LORA_MAX_NSYM];
} lora_tx_workspace;

typedef struct {
    uint32_t symbols[LORA_MAX_NSYM];
    /* Shared byte workspace: used sequentially as whitened -> dewhitened (payload+CRC)
       and for CRC computation to avoid extra buffers. */
    uint8_t bytes[LORA_MAX_NSYM];
#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex qchips[LORA_MAX_CHIPS];
#endif
    /* Persistent FFT demod workspace/context for embedded reuse (no malloc) */
    void *fft_ws;        /* caller-provided aligned workspace buffer */
    size_t fft_ws_size;  /* size of fft_ws in bytes */
    uint8_t fft_sf;      /* cached init params for reuse */
    uint32_t fft_fs;
    uint32_t fft_bw;
    int fft_inited;      /* whether fft_ctx has been initialised */
    struct lora_fft_demod_ctx *reserved_do_not_use_ptr; /* ABI padding */
    struct lora_fft_demod_ctx fft_ctx; /* embedded context storage */

    /* Sync/analysis metrics (populated by lora_rx_chain) */
    size_t sync_preamble_start; /* first preamble-like symbol index (best effort) */
    size_t sync_preamble_end;   /* index after preamble */
    size_t sync_sfd_end;        /* index after SFD window */
    size_t sync_sym_off;        /* symbols trimmed before decode */
    uint8_t sync_preamble_match_pct; /* 0..100 of preamble-like within span */
    uint8_t sync_sfd_nonzero;        /* count of non-preamble-like in SFD window */
    float   sync_cfo_hz;             /* estimated CFO in Hz (after clamp) */
} lora_rx_workspace;

/*
 * Caller provides output buffers sized at least by macros in lora_config.h.
 */
lora_status lora_tx_chain(const uint8_t *restrict payload, size_t payload_len,
                          float complex *restrict chips, size_t chips_buf_len,
                          size_t *restrict nchips_out,
                          const lora_chain_cfg *cfg,
                          lora_tx_workspace *ws);
lora_status lora_rx_chain(const float complex *restrict chips, size_t nchips,
                          uint8_t *restrict payload, size_t payload_buf_len,
                          size_t *restrict payload_len_out,
                          const lora_chain_cfg *cfg,
                          lora_rx_workspace *ws);

/* Helper: bytes required for FFT demod workspace for the given config. */
size_t lora_rx_fft_workspace_bytes(const lora_chain_cfg *cfg);

/* Helper: bytes required for TX workspace (constant for current layout). */
size_t lora_tx_workspace_bytes(const lora_chain_cfg *cfg);

lora_status lora_tx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);
lora_status lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);
