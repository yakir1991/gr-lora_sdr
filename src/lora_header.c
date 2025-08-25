#include "lora_header.h"


void lora_build_header(uint8_t payload_len, uint8_t cr, bool has_crc,
                       uint8_t header[5])
{
    header[0] = (payload_len >> 4) & 0x0F;
    header[1] = payload_len & 0x0F;
    header[2] = (uint8_t)((cr << 1) | (has_crc ? 1u : 0u));

    bool c4 = ((header[0] & 0x8) >> 3) ^ ((header[0] & 0x4) >> 2) ^
              ((header[0] & 0x2) >> 1) ^ (header[0] & 0x1);
    bool c3 = ((header[0] & 0x8) >> 3) ^ ((header[1] & 0x8) >> 3) ^
              ((header[1] & 0x4) >> 2) ^ ((header[1] & 0x2) >> 1) ^
              (header[2] & 0x1);
    bool c2 = ((header[0] & 0x4) >> 2) ^ ((header[1] & 0x8) >> 3) ^
              (header[1] & 0x1) ^ ((header[2] & 0x8) >> 3) ^
              ((header[2] & 0x2) >> 1);
    bool c1 = ((header[0] & 0x2) >> 1) ^ ((header[1] & 0x4) >> 2) ^
              (header[1] & 0x1) ^ ((header[2] & 0x4) >> 2) ^
              ((header[2] & 0x2) >> 1) ^ (header[2] & 0x1);
    bool c0 = (header[0] & 0x1) ^ ((header[1] & 0x2) >> 1) ^
              ((header[2] & 0x8) >> 3) ^ ((header[2] & 0x4) >> 2) ^
              ((header[2] & 0x2) >> 1) ^ (header[2] & 0x1);

    header[3] = (uint8_t)c4;
    header[4] = (uint8_t)((c3 << 3) | (c2 << 2) | (c1 << 1) | c0);
}

int lora_parse_header(const uint8_t header[5], uint8_t *payload_len,
                      uint8_t *cr, bool *has_crc)
{
    uint8_t len = (uint8_t)((header[0] << 4) | (header[1] & 0x0F));
    bool crc_flag = (header[2] & 0x1) != 0;
    uint8_t coding_rate = header[2] >> 1;

    bool c4 = ((header[0] & 0x8) >> 3) ^ ((header[0] & 0x4) >> 2) ^
              ((header[0] & 0x2) >> 1) ^ (header[0] & 0x1);
    bool c3 = ((header[0] & 0x8) >> 3) ^ ((header[1] & 0x8) >> 3) ^
              ((header[1] & 0x4) >> 2) ^ ((header[1] & 0x2) >> 1) ^
              (header[2] & 0x1);
    bool c2 = ((header[0] & 0x4) >> 2) ^ ((header[1] & 0x8) >> 3) ^
              (header[1] & 0x1) ^ ((header[2] & 0x8) >> 3) ^
              ((header[2] & 0x2) >> 1);
    bool c1 = ((header[0] & 0x2) >> 1) ^ ((header[1] & 0x4) >> 2) ^
              (header[1] & 0x1) ^ ((header[2] & 0x4) >> 2) ^
              ((header[2] & 0x2) >> 1) ^ (header[2] & 0x1);
    bool c0 = (header[0] & 0x1) ^ ((header[1] & 0x2) >> 1) ^
              ((header[2] & 0x8) >> 3) ^ ((header[2] & 0x4) >> 2) ^
              ((header[2] & 0x2) >> 1) ^ (header[2] & 0x1);

    uint8_t chk = (uint8_t)(((header[3] & 0x1) << 4) | (header[4] & 0x0F));
    uint8_t calc = (uint8_t)((c4 << 4) | (c3 << 3) | (c2 << 2) | (c1 << 1) | c0);

    if (chk != calc || len == 0)
        return -1;

    if (payload_len)
        *payload_len = len;
    if (cr)
        *cr = coding_rate;
    if (has_crc)
        *has_crc = crc_flag;

    return 0;
}

void rh_rf95_build_header(uint8_t to, uint8_t from, uint8_t id, uint8_t flags,
                          uint8_t header[4])
{
    header[0] = to;
    header[1] = from;
    header[2] = id;
    header[3] = flags;
}

void rh_rf95_parse_header(const uint8_t header[4], uint8_t *to,
                          uint8_t *from, uint8_t *id, uint8_t *flags)
{
    if (to)
        *to = header[0];
    if (from)
        *from = header[1];
    if (id)
        *id = header[2];
    if (flags)
        *flags = header[3];
}

