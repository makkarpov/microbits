#include <ub/crypto/utility.hpp>

#include <cstdint>

namespace c = ub::crypto;

void c::secureZero(void *buffer, size_t length) {
    auto p = (volatile uint8_t *) buffer;
    auto e = p + length;

    while (p != e) {
        *p = 0;
        p++;
    }
}

bool c::secureCompare(const void *a, const void *b, size_t length) {
    uint8_t diff = 0;

    for (size_t i = 0; i < length; i++) {
        uint8_t ai = ((const uint8_t *) a)[i];
        uint8_t bi = ((const uint8_t *) b)[i];
        diff |= ai ^ bi;
    }

    diff |= (diff >> 4);
    diff |= (diff >> 2);
    diff |= (diff >> 1);

    return (bool) ((diff ^ 1) & 1);
}
