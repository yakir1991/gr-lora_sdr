#include <stdio.h>
#include "signal_utils.h"

int main(void) {
    double samples[] = {1.0, -1.0, 1.0, -1.0};
    double value = rms(samples, 4);
    printf("RMS: %.1f\n", value);
    return value == 1.0 ? 0 : 1;
}
