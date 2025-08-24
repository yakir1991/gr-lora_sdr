#ifndef LORA_HEADER_H
#define LORA_HEADER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Build a LoRa PHY header using nibble values.
 * payload_len : payload length in bytes
 * cr          : coding rate (1..4)
 * has_crc     : true if payload has CRC
 * header      : 5-nibble output array
 */
void lora_build_header(uint8_t payload_len, uint8_t cr, bool has_crc,
                       uint8_t header[5]);

/* Parse a LoRa PHY header.
 * Returns 0 on success, -1 on CRC mismatch or invalid length.
 */
int lora_parse_header(const uint8_t header[5], uint8_t *payload_len,
                      uint8_t *cr, bool *has_crc);

/* Build a RadioHead-compatible header (to, from, id, flags).
 * header must have length 4 bytes.
 */
void rh_rf95_build_header(uint8_t to, uint8_t from, uint8_t id, uint8_t flags,
                          uint8_t header[4]);

/* Parse a RadioHead-compatible header (to, from, id, flags). */
void rh_rf95_parse_header(const uint8_t header[4], uint8_t *to,
                          uint8_t *from, uint8_t *id, uint8_t *flags);

#endif /* LORA_HEADER_H */
