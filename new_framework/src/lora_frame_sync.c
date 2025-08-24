#include "lora_frame_sync.h"

size_t lora_frame_sync_find_preamble(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len)
{
    size_t count = 0;
    for (size_t i = 0; i < nsym; ++i) {
        if (symbols[i] == 0) {
            count++;
            if (count >= preamble_len) {
                return i + 1; // index after the last preamble symbol
            }
        } else {
            count = 0;
        }
    }
    return nsym; // indicate not found
}

size_t lora_frame_sync_align(const uint32_t *symbols,
                             size_t nsym,
                             uint16_t preamble_len,
                             uint32_t *aligned)
{
    size_t start = lora_frame_sync_find_preamble(symbols, nsym, preamble_len);
    if (start >= nsym)
        return 0; // no preamble found

    size_t out_len = nsym - start;
    for (size_t i = 0; i < out_len; ++i)
        aligned[i] = symbols[start + i];
    return out_len;
}
