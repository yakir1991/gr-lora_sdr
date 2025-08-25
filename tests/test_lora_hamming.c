#include <assert.h>
#include <stdio.h>
#include "lora_hamming.h"

int main(void)
{
    uint8_t nibble = 0xA;
    struct { uint8_t cr; uint8_t expected; } cases[] = {
        {1, 0x14},
        {2, 0x16},
        {3, 0x2C},
        {4, 0x59},
    };

    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        uint8_t cr = cases[i].cr;
        uint8_t cw = lora_hamming_encode(nibble, cr);
        assert(cw == cases[i].expected);
        uint8_t dec = lora_hamming_decode(cw, cr);
        assert(dec == nibble);
        if (cr >= 3) {
            uint8_t cw_len = (cr == 1) ? 5 : (4 + cr);
            for (uint8_t b = 0; b < cw_len; ++b) {
                uint8_t flipped = cw ^ (1u << b);
                uint8_t corr = lora_hamming_decode(flipped, cr);
                assert(corr == nibble);
            }
        }
    }

    printf("All Hamming tests passed\n");
    return 0;
}

