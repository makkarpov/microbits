#ifndef UB_CRYPTO_UTILITY_H
#define UB_CRYPTO_UTILITY_H

#include <cstddef>

namespace ub::crypto {
    /** Securely zero memory block */
    void secureZero(void *buffer, size_t length);

    /** @return true if two buffers contain identical content */
    bool secureCompare(const void *a, const void *b, size_t length);
}

#endif // UB_CRYPTO_UTILITY_H
