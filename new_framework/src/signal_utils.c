#include "signal_utils.h"
#include <math.h>

/**
 * Compute the root mean square of an array of samples.
 */
double rms(const double *data, size_t len) {
    if (!data || len == 0) {
        return 0.0;
    }
    double sum = 0.0;
    for (size_t i = 0; i < len; ++i) {
        sum += data[i] * data[i];
    }
    return sqrt(sum / len);
}
