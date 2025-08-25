#include "lora_add_crc.h"
#include <stdio.h>
#include <string.h>
#include "lora_log.h"

int main(void)
{
    uint8_t crc[4];

    uint8_t payload1[] = {0x01, 0x02, 0x03, 0x04, 0x00, 0x00};
    uint8_t expected1[] = {0x3, 0x0, 0xD, 0x0};
    lora_add_crc(payload1, sizeof(payload1), crc);
    if (memcmp(crc, expected1, sizeof(expected1)) != 0) {
        LORA_LOG_INFO("CRC mismatch for payload1");
        return 1;
    }

    uint8_t payload2[] = {'H','e','l','l','o',0x00,0x00};
    uint8_t expected2[] = {0x6, 0xD, 0xB, 0xC};
    lora_add_crc(payload2, sizeof(payload2), crc);
    if (memcmp(crc, expected2, sizeof(expected2)) != 0) {
        LORA_LOG_INFO("CRC mismatch for payload2");
        return 1;
    }

    LORA_LOG_INFO("All CRC tests passed");
    return 0;
}

