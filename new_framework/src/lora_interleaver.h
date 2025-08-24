#ifndef LORA_INTERLEAVER_H
#define LORA_INTERLEAVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Interleave a block of LoRa codewords.
 *
 * in        : array of sf_app input codewords (each up to 8 bits)
 * out       : array of cw_len output symbols (each sf bits)
 * sf        : spreading factor (number of bits per output symbol)
 * sf_app    : number of codewords in the block (usually sf or sf-2)
 * cw_len    : number of bits per codeword (4+cr or 8 for header)
 * add_parity: when true, append parity bit at column sf_app
 */
void lora_interleave(const uint8_t *in, uint32_t *out,
                     uint8_t sf, uint8_t sf_app, uint8_t cw_len,
                     bool add_parity);

/*
 * Reverse operation of lora_interleave.
 *
 * in     : array of cw_len interleaved symbols
 * out    : recovered sf_app codewords
 * sf     : spreading factor
 * sf_app : number of codewords in block
 * cw_len : number of bits per codeword
 */
void lora_deinterleave(const uint32_t *in, uint8_t *out,
                       uint8_t sf, uint8_t sf_app, uint8_t cw_len);

#endif /* LORA_INTERLEAVER_H */
