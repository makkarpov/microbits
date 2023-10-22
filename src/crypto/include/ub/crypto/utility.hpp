#ifndef UB_CRYPTO_UTILITY_H
#define UB_CRYPTO_UTILITY_H

#include <cstddef>

namespace ub::crypto {
    /** Securely zero memory block */
    void secureZero(void *buffer, size_t length);

    /** @return true if two buffers contain identical content */
    bool secureCompare(const void *a, const void *b, size_t length);

    /**
     * Perform exclusive-or operation `dst[i] = src[i] ^ gamma[i]`.
     * `dst` buffer is used for `gamma` if `gamma` is null
     */
    void exclusiveOr(void *dst, const void *src, size_t length, const void *gamma = nullptr);
}

#endif // UB_CRYPTO_UTILITY_H
