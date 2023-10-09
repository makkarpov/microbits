#include "sha2_common.hpp"

#include <cstring>

constexpr static size_t BLOCK_SHA256 = 64;
constexpr static size_t TRAILER_SHA256 = 8;

bool ub::crypto::impl::writeSHA2Trailer(uint8_t *block, size_t used, size_t totalBytes, uint8_t k) {
    size_t blockLength = BLOCK_SHA256 << k;

    memset(block + used, 0, blockLength - used);
    if (used + (TRAILER_SHA256 << k) > blockLength) {
        return false;
    }

    // Support up to 4GB data length without integer overflow on 32-bit platforms:
    block[blockLength - 1] = (uint8_t) (totalBytes << 3);
    *((uint32_t *) (block + blockLength - 5)) = __builtin_bswap32(totalBytes >> 5);

    return true;
}
