#include "lora_add_crc.h"

static uint16_t crc16_step(uint16_t crc, uint8_t newByte)
{
    for (int i = 0; i < 8; i++) {
        if (((crc & 0x8000) >> 8) ^ (newByte & 0x80))
            crc = (uint16_t)((crc << 1) ^ 0x1021);
        else
            crc <<= 1;
        newByte <<= 1;
    }
    return crc;
}

void lora_add_crc(const uint8_t *payload, size_t length, uint8_t crc_nibbles[4])
{
    if (length < 2) {
        for (int i = 0; i < 4; i++)
            crc_nibbles[i] = 0;
        return;
    }

    uint16_t crc = 0x0000;
    size_t data_len = length - 2;
    for (size_t i = 0; i < data_len; i++)
        crc = crc16_step(crc, payload[i]);

    crc ^= payload[length - 1] ^ ((uint16_t)payload[length - 2] << 8);

    crc_nibbles[0] = crc & 0x0F;
    crc_nibbles[1] = (crc >> 4) & 0x0F;
    crc_nibbles[2] = (crc >> 8) & 0x0F;
    crc_nibbles[3] = (crc >> 12) & 0x0F;
}

