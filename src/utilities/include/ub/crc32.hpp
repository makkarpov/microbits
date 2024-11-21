#ifndef UB_UTILITIES_CRC32_H
#define UB_UTILITIES_CRC32_H

#include <cstdint>
#include <cstddef>

namespace ub {
    /** Compute checksum for a buffer */
    uint32_t crc32(const void *buffer, size_t length, uint32_t init = 0);
}

#endif // UB_UTILITIES_CRC32_H
