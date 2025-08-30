#include "lora_frame_sync.h"
#include <string.h>

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

static inline int is_preamble_sym_u32_cfg(uint32_t s, uint8_t tol) {
    return s <= (uint32_t)tol;
}

size_t lora_frame_sync_find_preamble(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len)
{
    if (!symbols || nsym == 0 || preamble_len == 0)
        return nsym;

    /* Minimal matches required within any window of length preamble_len. */
    size_t min_matches = (size_t)((preamble_len * LORA_FS_MIN_MATCH_PCT + 99) / 100); /* ceil */
    if (min_matches > preamble_len) min_matches = preamble_len;

    /* Sliding window over is_preamble flags to avoid backtracking. */
    size_t win = 0;
    size_t count = 0;
    for (; win < (size_t)preamble_len && win < nsym; ++win)
        count += is_preamble_sym_u32(symbols[win]);

    size_t i = 0;
    while (1) {
        if (count >= min_matches) {
            /* Extend to end of contiguous preamble-like region. */
            size_t end = (i + preamble_len <= nsym) ? (i + preamble_len) : nsym;
            while (end < nsym && is_preamble_sym_u32(symbols[end])) ++end;
            return end;
        }
        if (i + preamble_len >= nsym) break;
        /* Slide: remove left, add right */
        count -= is_preamble_sym_u32(symbols[i]);
        count += is_preamble_sym_u32(symbols[i + preamble_len]);
        ++i;
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

    /* Adaptive sliding of an sfd_len window within [start, start+lookahead].
     * Accept the first window that has at least LORA_FS_SFD_MIN_NONZERO non‑preamble symbols. */
    size_t start = preamble_end;
    size_t max_start = start + lookahead;
    if (start + sfd_len > nsym) return preamble_end;
    if (max_start + sfd_len > nsym) {
        if (nsym > sfd_len) max_start = nsym - sfd_len; else return preamble_end;
    }
    /* Initialize count for the first window */
    size_t end = start + sfd_len;
    size_t nonzero = 0;
    for (size_t i = start; i < end; ++i) nonzero += !is_preamble_sym_u32(symbols[i]);
    if (nonzero >= (size_t)LORA_FS_SFD_MIN_NONZERO) return end;
    /* Slide the window */
    while (start < max_start) {
        /* remove left, add right */
        nonzero -= !is_preamble_sym_u32(symbols[start]);
        ++start; ++end;
        nonzero += !is_preamble_sym_u32(symbols[end - 1]);
        if (nonzero >= (size_t)LORA_FS_SFD_MIN_NONZERO)
            return end;
    }
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

size_t lora_frame_sync_align_offset(const uint32_t *symbols,
                                    size_t nsym,
                                    uint16_t preamble_len,
                                    size_t *offset_out)
{
    if (offset_out) *offset_out = nsym;
    size_t start = lora_frame_sync_find_preamble(symbols, nsym, preamble_len);
    if (start >= nsym)
        return 0;
    if (offset_out) *offset_out = start;
    return nsym - start;
}

size_t lora_frame_sync_analyze(const uint32_t *symbols,
                               size_t nsym,
                               uint16_t preamble_len,
                               uint8_t sfd_len,
                               uint8_t lookahead,
                               lora_fs_metrics *out)
{
    if (out) memset(out, 0, sizeof(*out));
    size_t pre_end = lora_frame_sync_find_preamble(symbols, nsym, preamble_len);
    size_t pre_start = pre_end;
    /* Walk back to approximate start of preamble-like run */
    while (pre_start > 0 && is_preamble_sym_u32(symbols[pre_start - 1])) --pre_start;
    size_t sfd_end = lora_frame_sync_find_sfd(symbols, nsym, pre_end, sfd_len, lookahead);

    if (out) {
        out->preamble_start = pre_start;
        out->preamble_end = pre_end;
        out->sfd_end = sfd_end;
        uint8_t pct = 0, nonz = 0;
        size_t span = (pre_end > pre_start) ? (pre_end - pre_start) : 0;
        if (span > 0 && span < 255) {
            size_t good = 0;
            for (size_t i = pre_start; i < pre_end; ++i) good += is_preamble_sym_u32(symbols[i]);
            pct = (uint8_t)((100u * good + (span/2)) / span);
        }
        if (sfd_end > pre_end) {
            size_t sfdw = sfd_end - pre_end;
            for (size_t i = pre_end; i < sfd_end; ++i) nonz += !is_preamble_sym_u32(symbols[i]);
            if (sfdw > 0 && nonz > 255) nonz = 255;
        }
        out->preamble_match_pct = pct;
        out->sfd_nonzero = nonz;
    }
    return sfd_end;
}

void lora_frame_sync_recommend(uint8_t sf, uint32_t bw, lora_fs_cfg *out)
{
    if (!out) return;
    /* Base defaults */
    out->tol_bin = 1;               /* treat 0..1 as preamble-like */
    out->min_match_pct = 80;        /* 80% of window must match */
    out->sfd_len = 2;               /* typical SFD span */
    out->sfd_min_nonzero = 2;       /* both should be nonzero-like */
    /* Heuristic lookahead: a bit larger for higher SFs */
    (void)bw;
    out->lookahead = (sf >= 11) ? 6 : (sf >= 9 ? 4 : 2);
}

size_t lora_frame_sync_find_preamble_cfg(const uint32_t *symbols,
                                         size_t nsym,
                                         uint16_t preamble_len,
                                         const lora_fs_cfg *cfg)
{
    if (!symbols || nsym == 0 || preamble_len == 0 || !cfg)
        return nsym;
    uint8_t tol = cfg->tol_bin;
    uint8_t min_pct = cfg->min_match_pct ? cfg->min_match_pct : LORA_FS_MIN_MATCH_PCT;
    size_t min_matches = (size_t)((preamble_len * min_pct + 99) / 100);
    if (min_matches > preamble_len) min_matches = preamble_len;

    size_t win = 0, count = 0;
    for (; win < (size_t)preamble_len && win < nsym; ++win)
        count += is_preamble_sym_u32_cfg(symbols[win], tol);
    size_t i = 0;
    while (1) {
        if (count >= min_matches) {
            size_t end = (i + preamble_len <= nsym) ? (i + preamble_len) : nsym;
            while (end < nsym && is_preamble_sym_u32_cfg(symbols[end], tol)) ++end;
            return end;
        }
        if (i + preamble_len >= nsym) break;
        count -= is_preamble_sym_u32_cfg(symbols[i], tol);
        count += is_preamble_sym_u32_cfg(symbols[i + preamble_len], tol);
        ++i;
    }
    return nsym;
}

size_t lora_frame_sync_find_sfd_cfg(const uint32_t *symbols,
                                    size_t nsym,
                                    size_t preamble_end,
                                    const lora_fs_cfg *cfg)
{
    if (!symbols || preamble_end >= nsym || !cfg)
        return preamble_end;
    uint8_t sfd_len = cfg->sfd_len ? cfg->sfd_len : LORA_FS_SFD_LEN;
    uint8_t lookahead = cfg->lookahead ? cfg->lookahead : LORA_FS_SFD_LOOKAHEAD;
    uint8_t nonzero_req = cfg->sfd_min_nonzero ? cfg->sfd_min_nonzero : LORA_FS_SFD_MIN_NONZERO;

    size_t start = preamble_end;
    size_t max_start = start + lookahead;
    if (start + sfd_len > nsym) return preamble_end;
    if (max_start + sfd_len > nsym) {
        if (nsym > sfd_len) max_start = nsym - sfd_len; else return preamble_end;
    }
    size_t end = start + sfd_len;
    size_t nonzero = 0;
    for (size_t i = start; i < end; ++i) nonzero += !is_preamble_sym_u32_cfg(symbols[i], cfg->tol_bin);
    if (nonzero >= nonzero_req) return end;
    while (start < max_start) {
        nonzero -= !is_preamble_sym_u32_cfg(symbols[start], cfg->tol_bin);
        ++start; ++end;
        nonzero += !is_preamble_sym_u32_cfg(symbols[end - 1], cfg->tol_bin);
        if (nonzero >= nonzero_req) return end;
    }
    return preamble_end;
}
