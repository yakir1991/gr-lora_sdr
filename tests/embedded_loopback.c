#include <string.h>
#include <stdint.h>
#include <complex.h>
#include "lora_log.h"
#include "lora_chain.h"
#include "lora_config.h"

typedef struct { int16_t r; int16_t i; } q15c;

static int16_t to_q15(float x) {
    if (x > 0.999969f) x = 0.999969f;
    if (x < -1.0f) x = -1.0f;
    return (int16_t)(x * 32767.0f + (x >= 0 ? 0.5f : -0.5f));
}

static const q15c expected_chips[32] = {
    {32766, 0},
    {1206, 32745},
    {-32728, 1608},
    {-1206, -32745},
    {32766, 0},
    {-2009, 32705},
    {-32412, -4808},
    {8351, -31685},
    {30273, 12539},
    {-17189, 27896},
    {-24279, -22005},
    {26556, -19195},
    {12539, 30273},
    {-32469, 4410},
    {4808, -32412},
    {29447, 14372},
    {-23170, 23170},
    {-13645, -29791},
    {32728, -1608},
    {-11417, 30714},
    {-23170, -23170},
    {30985, -10659},
    {-4808, 32412},
    {-26077, -19841},
    {30273, -12539},
    {-5205, 32351},
    {-24279, -22005},
    {31880, -7571},
    {-12539, 30273},
    {-16499, -28310},
    {32412, 4808},
    {-24547, 21705}
};

int main(void) {
    static const uint8_t payload[1] = {0x41};
    static float complex chips[LORA_MAX_CHIPS];
    static uint8_t rx[LORA_MAX_PAYLOAD_LEN];
    size_t nchips = 0, rx_len = 0;

    if (lora_tx_chain(payload, sizeof(payload), chips, LORA_MAX_CHIPS, &nchips) != 0) {
        LORA_LOG_ERR("TX chain failed");
        return 1;
    }
    if (nchips != 768) {
        LORA_LOG_ERR("Unexpected chip count %zu", nchips);
        return 1;
    }
    for (size_t i = 0; i < 32; ++i) {
        q15c q = { to_q15(crealf(chips[i])), to_q15(cimagf(chips[i])) };
        if (q.r != expected_chips[i].r || q.i != expected_chips[i].i) {
            LORA_LOG_ERR("Chip mismatch at %zu", i);
            return 1;
        }
    }
    if (lora_rx_chain(chips, nchips, rx, sizeof(rx), &rx_len) != 0) {
        LORA_LOG_ERR("RX chain failed");
        return 1;
    }
    if (rx_len != sizeof(payload) || memcmp(rx, payload, sizeof(payload)) != 0) {
        LORA_LOG_ERR("Payload mismatch");
        return 1;
    }
    LORA_LOG_INFO("Embedded loopback test passed");
    return 0;
}
