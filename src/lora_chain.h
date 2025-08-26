#pragma once

#include <stddef.h>
#include <stdint.h>
#include <complex.h>
#include "lora_config.h"
#include "lora_io.h"

typedef struct {
    uint8_t sf;
    uint32_t bw;
    uint32_t samp_rate;
} lora_chain_cfg;

/*
 * Caller provides output buffers sized at least by macros in lora_config.h.
 */
int lora_tx_chain(const uint8_t *restrict payload, size_t payload_len,
                  float complex *restrict chips, size_t chips_buf_len,
                  size_t *restrict nchips_out,
                  const lora_chain_cfg *cfg);
int lora_rx_chain(const float complex *restrict chips, size_t nchips,
                  uint8_t *restrict payload, size_t payload_buf_len,
                  size_t *restrict payload_len_out,
                  const lora_chain_cfg *cfg);

int lora_tx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);
int lora_rx_run(lora_io_t *in, lora_io_t *out, const lora_chain_cfg *cfg);

