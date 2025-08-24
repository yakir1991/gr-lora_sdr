#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Compute the LoRa payload CRC as implemented in gr-lora_sdr add_crc block.
 *
 * The input payload should contain the user data followed by two bytes
 * reserved for the CRC. The computed CRC is XORed with these two bytes and
 * returned as four nibble values (least significant nibble first).
 *
 * \param payload   Pointer to payload bytes including two CRC bytes.
 * \param length    Number of bytes in payload (including CRC bytes).
 * \param crc_nibbles Output array of four bytes where nibble values are stored.
 */
void lora_add_crc(const uint8_t *payload, size_t length, uint8_t crc_nibbles[4]);

#ifdef __cplusplus
}
#endif

