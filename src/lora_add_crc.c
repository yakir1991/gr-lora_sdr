#include "lora_add_crc.h"

#include "lora_crc_table.h"

void lora_add_crc(const uint8_t *restrict payload, size_t length, uint8_t crc_nibbles[4])
{
    if (length < 2) {
        crc_nibbles[0] = crc_nibbles[1] = crc_nibbles[2] = crc_nibbles[3] = 0;
        return;
    }

    uint16_t crc = 0x0000; /* match legacy init */
    size_t data_len = length - 2;
    size_t i = 0;
    /* Unroll by 4 to reduce loop overhead on small MCUs */
    size_t n4 = data_len & ~(size_t)3;
    for (; i < n4; i += 4) {
        uint8_t idx0 = (uint8_t)((crc >> 8) ^ payload[i + 0]);
        crc = (uint16_t)((crc << 8) ^ LORA_CRC16_CCITT_TAB[idx0]);
        uint8_t idx1 = (uint8_t)((crc >> 8) ^ payload[i + 1]);
        crc = (uint16_t)((crc << 8) ^ LORA_CRC16_CCITT_TAB[idx1]);
        uint8_t idx2 = (uint8_t)((crc >> 8) ^ payload[i + 2]);
        crc = (uint16_t)((crc << 8) ^ LORA_CRC16_CCITT_TAB[idx2]);
        uint8_t idx3 = (uint8_t)((crc >> 8) ^ payload[i + 3]);
        crc = (uint16_t)((crc << 8) ^ LORA_CRC16_CCITT_TAB[idx3]);
    }
    for (; i < data_len; ++i) {
        uint8_t idx = (uint8_t)((crc >> 8) ^ payload[i]);
        crc = (uint16_t)((crc << 8) ^ LORA_CRC16_CCITT_TAB[idx]);
    }

    /* Preserve legacy post-xor with trailing bytes (usually zeros). */
    crc ^= (uint16_t)payload[length - 1] ^ ((uint16_t)payload[length - 2] << 8);

    crc_nibbles[0] = (uint8_t)(crc & 0x0F);
    crc_nibbles[1] = (uint8_t)((crc >> 4) & 0x0F);
    crc_nibbles[2] = (uint8_t)((crc >> 8) & 0x0F);
    crc_nibbles[3] = (uint8_t)((crc >> 12) & 0x0F);
}
