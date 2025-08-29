#ifndef LORA_FRAME_SYNC_H
#define LORA_FRAME_SYNC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Detect the start index of a LoRa frame preamble within a stream of symbol
 * values. This implementation is tolerant to occasional errors: a symbol is
 * considered a preamble symbol if its value is within a small tolerance of 0
 * (configurable at compile time), and a preamble window may include a small
 * fraction of non‑preamble symbols (also configurable).
 *
 * By default it:
 *  - Treats symbol values 0..LORA_FS_TOL_BIN as preamble symbols
 *  - Accepts windows with at least LORA_FS_MIN_MATCH_PCT percent matches
 *
 * The function returns the index of the first symbol following the detected
 * preamble (i.e. the end of the full preamble run, which can be longer than
 * 'preamble_len'). If no preamble is found, the function returns nsym.
 */
size_t lora_frame_sync_find_preamble(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len);

/*
 * Validate and refine the frame start by detecting an SFD-like transition
 * immediately following the preamble. Operating in symbol domain, this checks
 * that the next 'sfd_len' symbols after 'preamble_end' contain enough
 * non‑preamble symbols (not near zero) to mark the end of the preamble/SFD
 * region. Returns the index immediately after the SFD if detected. If no SFD
 * evidence is found within the lookahead window, returns 'preamble_end'.
 */
size_t lora_frame_sync_find_sfd(const uint32_t *symbols,
                                size_t nsym,
                                size_t preamble_end,
                                uint8_t sfd_len,
                                uint8_t lookahead);

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
