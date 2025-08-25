#include <stdio.h>
#include "lora_log.h"
#include "signal_utils.h"

int main(void) {
    double samples[] = {1.0, -1.0, 1.0, -1.0};
    double value = rms(samples, 4);
    LORA_LOG_INFO("RMS: %.1f", value);
    return value == 1.0 ? 0 : 1;
}
