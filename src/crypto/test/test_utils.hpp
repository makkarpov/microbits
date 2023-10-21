#ifndef UB_TEST_CRYPTO_TEST_UTILS_H
#define UB_TEST_CRYPTO_TEST_UTILS_H

#include <cstdint>
#include <cstddef>

/** Print 256-bit number stored in little-endian form */
void printNumber256(const char *prefix, const uint8_t *x);

/** Print 256-bit number stored in little-endian form */
void printNumber448(const char *prefix, const uint8_t *x);

/** Print raw bytes as hex */
void printBytes(const char *prefix, const uint8_t *x, size_t length);

#endif // UB_TEST_CRYPTO_TEST_UTILS_H
