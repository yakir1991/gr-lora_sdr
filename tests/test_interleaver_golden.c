#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lora_log.h"
#include "lora_interleaver.h"

int main(void)
{
    const uint8_t cw_len = 8;
    static const uint32_t golden[6][8] = {
        {0, 0, 0, 12, 65, 88, 25, 85},
        {0, 0, 0, 28, 129, 180, 51, 170},
        {0, 0, 0, 60, 273, 356, 99, 342},
        {0, 0, 0, 124, 561, 724, 195, 682},
        {0, 0, 0, 252, 1137, 1460, 403, 1370},
        {0, 0, 512, 252, 2161, 2868, 819, 2730}
    };

    for (uint8_t sf = 7; sf <= 12; ++sf) {
        uint8_t sf_app = sf;
        uint8_t input[16];
        uint32_t inter[cw_len];
        for (uint8_t i = 0; i < sf_app; ++i) {
            input[i] = i * 3 + 1;
        }
        lora_interleave(input, inter, sf, sf_app, cw_len, false);
        if (memcmp(inter, golden[sf - 7], sizeof(uint32_t) * cw_len) != 0) {
            LORA_LOG_INFO("Interleaver golden test failed for SF %u", sf);
            return 1;
        }
    }
    LORA_LOG_INFO("Interleaver golden test passed");
    return 0;
}
