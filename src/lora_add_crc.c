#include "lora_add_crc.h"

/* Table-driven CRC-16-CCITT (poly 0x1021), same semantics as the legacy
 * bitwise implementation (init=0x0000, no reflect). */
static void crc16_table_init(uint16_t tab[256]) {
    for (int i = 0; i < 256; ++i) {
        uint16_t crc = (uint16_t)i << 8;
        for (int b = 0; b < 8; ++b)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        tab[i] = crc;
    }
}

void lora_add_crc(const uint8_t *payload, size_t length, uint8_t crc_nibbles[4])
{
    if (length < 2) {
        crc_nibbles[0] = crc_nibbles[1] = crc_nibbles[2] = crc_nibbles[3] = 0;
        return;
    }

    static uint16_t table[256];
    static int table_init = 0;
    if (!table_init) { crc16_table_init(table); table_init = 1; }

    uint16_t crc = 0x0000; /* match legacy init */
    size_t data_len = length - 2;
    for (size_t i = 0; i < data_len; ++i) {
        uint8_t idx = (uint8_t)((crc >> 8) ^ payload[i]);
        crc = (uint16_t)((crc << 8) ^ table[idx]);
    }

    /* Preserve legacy post-xor with trailing bytes (usually zeros). */
    crc ^= (uint16_t)payload[length - 1] ^ ((uint16_t)payload[length - 2] << 8);

    crc_nibbles[0] = (uint8_t)(crc & 0x0F);
    crc_nibbles[1] = (uint8_t)((crc >> 4) & 0x0F);
    crc_nibbles[2] = (uint8_t)((crc >> 8) & 0x0F);
    crc_nibbles[3] = (uint8_t)((crc >> 12) & 0x0F);
}
