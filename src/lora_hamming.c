#include "lora_hamming.h"

uint8_t lora_hamming_encode(uint8_t nibble, uint8_t cr)
{
    uint8_t d0 = nibble & 1u;
    uint8_t d1 = (nibble >> 1) & 1u;
    uint8_t d2 = (nibble >> 2) & 1u;
    uint8_t d3 = (nibble >> 3) & 1u;

    if (cr != 1) {
        uint8_t p0 = d0 ^ d1 ^ d2;
        uint8_t p1 = d1 ^ d2 ^ d3;
        uint8_t p2 = d0 ^ d1 ^ d3;
        uint8_t p3 = d0 ^ d2 ^ d3;
        uint8_t cw = (d0 << 7) | (d1 << 6) | (d2 << 5) | (d3 << 4) |
                     (p0 << 3) | (p1 << 2) | (p2 << 1) | p3;
        return cw >> (4 - cr);
    } else {
        uint8_t p4 = d0 ^ d1 ^ d2 ^ d3;
        return (d3 << 4) | (d2 << 3) | (d1 << 2) | (d0 << 1) | p4;
    }
}

uint8_t lora_hamming_decode(uint8_t codeword, uint8_t cr)
{
    if (cr == 1) {
        uint8_t d3 = (codeword >> 4) & 1u;
        uint8_t d2 = (codeword >> 3) & 1u;
        uint8_t d1 = (codeword >> 2) & 1u;
        uint8_t d0 = (codeword >> 1) & 1u;
        return (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
    }

    uint8_t cw_len = 4 + cr;
    uint8_t shift = 8 - cw_len;
    uint8_t cw = codeword << shift;

    uint8_t d0 = (cw >> 7) & 1u;
    uint8_t d1 = (cw >> 6) & 1u;
    uint8_t d2 = (cw >> 5) & 1u;
    uint8_t d3 = (cw >> 4) & 1u;
    uint8_t p0 = (cw >> 3) & 1u;
    uint8_t p1 = (cw >> 2) & 1u;
    uint8_t p2 = (cw >> 1) & 1u;
    uint8_t p3 = cw & 1u;

    if (cr == 4) {
        uint8_t parity = d0 ^ d1 ^ d2 ^ d3 ^ p0 ^ p1 ^ p2 ^ p3;
        if ((parity & 1u) == 0u)
            return (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
    }

    if (cr >= 3) {
        uint8_t s0 = d0 ^ d1 ^ d2 ^ p0;
        uint8_t s1 = d1 ^ d2 ^ d3 ^ p1;
        uint8_t s2 = d0 ^ d1 ^ d3 ^ p2;
        uint8_t synd = s0 | (s1 << 1) | (s2 << 2);
        switch (synd) {
        case 5:
            d0 ^= 1u;
            break;
        case 7:
            d1 ^= 1u;
            break;
        case 3:
            d2 ^= 1u;
            break;
        case 6:
            d3 ^= 1u;
            break;
        default:
            break;
        }
    }

    return (d3 << 3) | (d2 << 2) | (d1 << 1) | d0;
}

