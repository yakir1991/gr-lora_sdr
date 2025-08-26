#pragma once

#include <stddef.h>
#include <stdint.h>
#include <complex.h>
#include "lora_config.h"
#include "lora_io.h"
#ifdef LORA_LITE_FIXED_POINT
#include "lora_fixed.h"
#endif

typedef struct {
    uint8_t sf;
    uint32_t bw;
    uint32_t samp_rate;
} lora_chain_cfg;

typedef struct {
    uint8_t buf[LORA_MAX_PAYLOAD_LEN + 2];
    uint8_t whitened[LORA_MAX_PAYLOAD_LEN + 2];
    uint32_t symbols[LORA_MAX_NSYM];
} lora_tx_workspace;

typedef struct {
    uint32_t symbols[LORA_MAX_NSYM];
    uint8_t whitened[LORA_MAX_NSYM];
    uint8_t payload_crc[LORA_MAX_NSYM];
    uint8_t tmp[LORA_MAX_NSYM];
#ifdef LORA_LITE_FIXED_POINT
    lora_q15_complex qchips[LORA_MAX_CHIPS];
#endif
} lora_rx_workspace;

/*
 * Caller provides output buffers sized at least by macros in lora_config.h.
 */
int lora_tx_chain(const uint8_t *restrict payload, size_t payload_len,
                  float complex *restrict chips, size_t chips_buf_len,
                  size_t *restrict nchips_out,
                  const lora_chain_cfg *cfg,
                  lora_tx_workspace *ws);
int lora_rx_chain(const float complex *restrict chips, size_t nchips,
                  uint8_t *restrict payload, size_t payload_buf_len,
                  size_t *restrict payload_len_out,
                  const lora_chain_cfg *cfg,
                  lora_rx_workspace *ws);

int lora_tx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);
int lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);

