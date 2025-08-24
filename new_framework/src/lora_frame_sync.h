#ifndef LORA_FRAME_SYNC_H
#define LORA_FRAME_SYNC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Detect the start index of a LoRa frame preamble within a stream of symbol
 * values. The function searches for 'preamble_len' consecutive zero-valued
 * symbols and returns the index of the first symbol following the preamble.
 * If no preamble is found, the function returns the total number of symbols
 * provided (nsym).
 */
size_t lora_frame_sync_find_preamble(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len);

/*
 * Copy the portion of 'symbols' that follows the detected preamble into
 * 'aligned'. The function returns the number of symbols copied. If the preamble
 * is not found, zero is returned and 'aligned' is left untouched.
 */
size_t lora_frame_sync_align(const uint32_t *symbols,
                             size_t nsym,
                             uint16_t preamble_len,
                             uint32_t *aligned);

#ifdef __cplusplus
}
#endif

#endif /* LORA_FRAME_SYNC_H */
