/*
 * ro_tables.c — consolidation TU for read-only tables.
 *
 * This translation unit serves as the anchor for large read-only tables
 * (per-SF/CR LUTs, whitening tables, etc.) to improve icache locality and
 * make it easy to gate inclusion via build flags.
 *
 * Current tables already in rodata:
 *  - Gray map/demap: defined in `src/lora_gray_lut.h` (static-in-header).
 *  - CRC table:       defined in `src/lora_crc_table.h`.
 *  - Whitening LUT8:  optional static variant in `src/whiten_lut8_static.c`.
 *
 * Future tables may be moved here as single definitions to avoid duplication
 * across TUs and to enable LTO/GC-sections to operate predictably.
 */

#include <stdint.h>

#if defined(LORA_LITE_WHITEN_LUT8_STATIC)
/* Reference the whitening tables so they are linked under this TU. */
extern const uint64_t LORA_WHITEN_LUT8_STATIC[256];
extern const uint8_t  LORA_WHITEN_NEXT8_STATIC[256];
const void *lora_ro_anchor_whiten_lut8 = &LORA_WHITEN_LUT8_STATIC[0];
const void *lora_ro_anchor_whiten_next8 = &LORA_WHITEN_NEXT8_STATIC[0];
#endif

/* Placeholder anchors for future ro tables */
const void *lora_ro_anchor_reserved0 = 0;
const void *lora_ro_anchor_reserved1 = 0;

