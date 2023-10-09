#ifndef UB_SRC_CRYPTO_HASH_SHA2_COMMON_H
#define UB_SRC_CRYPTO_HASH_SHA2_COMMON_H

#include <cstdint>
#include <cstddef>

namespace ub::crypto::impl {
    enum {
        K_SHA256 = 0,
        K_SHA512 = 1
    };

    /**
     * Write SHA2 length trailer field (number of processed bits as big-endian). Returns true if field was written,
     * and false if no space is available and another block is needed.
     *
     * @param block      Hash block buffer
     * @param used       Number of used bytes in the block
     * @param totalBytes Total number of processed bytes
     * @param k          Parameter to select between SHA-256 and SHA-512 trailers, must be given by K_SHAxxx constant.
     */
    bool writeSHA2Trailer(uint8_t *block, size_t used, size_t totalBytes, uint8_t k);
}

#endif // UB_SRC_CRYPTO_HASH_SHA2_COMMON_H
