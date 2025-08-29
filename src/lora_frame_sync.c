#include "lora_frame_sync.h"

#ifndef LORA_FS_TOL_BIN
#define LORA_FS_TOL_BIN 1    /* consider 0..TOL as preamble-like */
#endif

#ifndef LORA_FS_MIN_MATCH_PCT
#define LORA_FS_MIN_MATCH_PCT 80 /* require at least this % matches in window */
#endif

/* Helper: classify preamble-like symbols (near zero). */
static inline int is_preamble_sym_u32(uint32_t s) {
    return s <= (uint32_t)LORA_FS_TOL_BIN;
}

size_t lora_frame_sync_find_preamble(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len)
{
    if (!symbols || nsym == 0 || preamble_len == 0)
        return nsym;

    /* Compute minimal number of matches required in a preamble_len window. */
    size_t min_matches = (size_t)((preamble_len * LORA_FS_MIN_MATCH_PCT + 99) / 100); /* ceil */
    if (min_matches > preamble_len) min_matches = preamble_len;
    size_t max_misses = (size_t)preamble_len - min_matches;

    /* Scan linearly, allowing up to max_misses non-preamble symbols inside a run. */
    size_t run_start = 0;
    size_t good = 0;
    size_t miss = 0;
    int in_run = 0;

    for (size_t i = 0; i < nsym; ++i) {
        int is_pre = is_preamble_sym_u32(symbols[i]);
        if (!in_run) {
            /* Start a run at the first preamble-like symbol we see. */
            if (is_pre) {
                in_run = 1;
                run_start = i;
                good = 1;
                miss = 0;
            }
            continue;
        }

        if (is_pre) {
            ++good;
        } else if (miss < max_misses) {
            ++miss;
        } else {
            /* Too many misses; reset and re-evaluate this position as a potential start. */
            in_run = 0;
            good = 0;
            miss = 0;
            --i; /* re-check current position for starting a new run */
            continue;
        }

        size_t span = (i - run_start + 1);
        if (span >= preamble_len && good >= min_matches) {
            /* Found sufficient preamble evidence; extend to the end of the preamble-like region. */
            size_t end = i + 1;
            while (end < nsym && is_preamble_sym_u32(symbols[end]))
                ++end;
            return end;
        }
    }

    return nsym;
}

/* Default parameters for SFD detection in symbol domain */
#ifndef LORA_FS_SFD_LEN
#define LORA_FS_SFD_LEN 2
#endif
#ifndef LORA_FS_SFD_MIN_NONZERO
#define LORA_FS_SFD_MIN_NONZERO 2
#endif
#ifndef LORA_FS_SFD_LOOKAHEAD
#define LORA_FS_SFD_LOOKAHEAD 4
#endif

size_t lora_frame_sync_find_sfd(const uint32_t *symbols,
                                size_t nsym,
                                size_t preamble_end,
                                uint8_t sfd_len,
                                uint8_t lookahead)
{
    if (!symbols || preamble_end >= nsym)
        return preamble_end;
    if (sfd_len == 0)
        sfd_len = LORA_FS_SFD_LEN;
    if (lookahead == 0)
        lookahead = LORA_FS_SFD_LOOKAHEAD;

    /* Evaluate SFD window immediately after preamble_end only. */
    size_t start = preamble_end;
    size_t end = start + sfd_len;
    if (end > nsym) return preamble_end;
    size_t nonzero = 0;
    for (size_t i = start; i < end; ++i)
        nonzero += !is_preamble_sym_u32(symbols[i]);
    if (nonzero >= (size_t)LORA_FS_SFD_MIN_NONZERO)
        return end;
    return preamble_end;
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
