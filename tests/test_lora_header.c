#include "lora_header.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lora_log.h"

int main(void)
{
    uint8_t hdr[5];
    uint8_t len, cr;
    bool crc;

    lora_build_header(13, 1, true, hdr);
    if (lora_parse_header(hdr, &len, &cr, &crc) != 0 || len != 13 || cr != 1 || !crc) {
        LORA_LOG_INFO("Header test 1 failed");
        return 1;
    }

    lora_build_header(42, 2, false, hdr);
    if (lora_parse_header(hdr, &len, &cr, &crc) != 0 || len != 42 || cr != 2 || crc) {
        LORA_LOG_INFO("Header test 2 failed");
        return 1;
    }

    lora_build_header(20, 1, true, hdr);
    hdr[4] ^= 0x1; // corrupt CRC
    if (lora_parse_header(hdr, &len, &cr, &crc) == 0) {
        LORA_LOG_INFO("Header CRC check failed");
        return 1;
    }

    uint8_t rh[4];
    uint8_t to, from, id, flags;
    rh_rf95_build_header(1, 2, 3, 4, rh);
    rh_rf95_parse_header(rh, &to, &from, &id, &flags);
    if (to != 1 || from != 2 || id != 3 || flags != 4) {
        LORA_LOG_INFO("RH_RF95 header test failed");
        return 1;
    }

    LORA_LOG_INFO("All header tests passed");
    return 0;
}
