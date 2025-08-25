#include <stdio.h>
#include <string.h>
#include "lora_interleaver.h"

int main(void)
{
    const uint8_t sf = 7;
    const uint8_t sf_app = 7;
    const uint8_t cw_len = 8;
    uint8_t input[sf_app];
    uint32_t inter[cw_len];
    uint8_t output[sf_app];

    for (uint8_t i = 0; i < sf_app; ++i)
        input[i] = i * 3 + 1;

    lora_interleave(input, inter, sf, sf_app, cw_len, false);
    lora_deinterleave(inter, output, sf, sf_app, cw_len);

    if (memcmp(input, output, sf_app) != 0) {
        printf("Interleaver test failed\n");
        return 1;
    }

    printf("Interleaver test passed\n");
    return 0;
}
