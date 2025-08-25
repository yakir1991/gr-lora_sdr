#include <stdio.h>
#include <string.h>
#include "lora_log.h"
#include "lora_whitening.h"

int main(void)
{
    const uint8_t packet[] = {'H','e','l','l','o',' ','w','o','r','l','d'};
    const size_t len = sizeof(packet);
    uint8_t whitened[len];
    uint8_t dewhitened[len];

    lora_whiten(packet, whitened, len);
    lora_dewhiten(whitened, dewhitened, len);

    if (memcmp(packet, dewhitened, len) != 0) {
        LORA_LOG_INFO("Whitening test failed");
        return 1;
    }

    LORA_LOG_INFO("Whitening test passed");
    return 0;
}
