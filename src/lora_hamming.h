#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Encode a 4-bit nibble using LoRa's Hamming code.
// cr values: 1->4/5, 2->4/6, 3->4/7, 4->4/8
uint8_t lora_hamming_encode(uint8_t nibble, uint8_t cr);

// Decode a LoRa Hamming codeword. Returns the original 4-bit nibble.
uint8_t lora_hamming_decode(uint8_t codeword, uint8_t cr);

#ifdef __cplusplus
}
#endif

