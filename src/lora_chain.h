#pragma once

#include <stddef.h>
#include <stdint.h>
#include <complex.h>
#include "lora_config.h"
#include "lora_io.h"

/*
 * Caller provides output buffers sized at least by macros in lora_config.h.
 */
int lora_tx_chain(const uint8_t *payload, size_t payload_len,
                  float complex *chips, size_t chips_buf_len,
                  size_t *nchips_out);
int lora_rx_chain(const float complex *chips, size_t nchips,
                  uint8_t *payload, size_t payload_buf_len,
                  size_t *payload_len_out);

int lora_tx_run(lora_io_t *in, lora_io_t *out);
int lora_rx_run(lora_io_t *in, lora_io_t *out);

