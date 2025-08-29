#ifndef SIGNAL_UTILS_H
#define SIGNAL_UTILS_H

#include <stddef.h>

/**
 * @brief Compute the root mean square of an array of samples.
 *
 * @param data Pointer to an array of double samples.
 * @param len  Number of elements in the array.
 * @return RMS value of the samples or 0.0 if data is NULL or len is zero.
 */
double rms(const double *data, size_t len);

/* Optimized single-precision RMS for embedded targets. Uses compensated
 * summation to improve numerical stability while keeping float math. */
float rmsf(const float *data, size_t len);

#endif // SIGNAL_UTILS_H
