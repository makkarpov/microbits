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

void ub::crypto::exclusiveOr(void *dst, const void *src, size_t length, const void *gamma) {
    auto dst8 = (uint8_t *) dst;
    auto src8 = (const uint8_t *) src;
    auto gamma8 = (const uint8_t *) (gamma != nullptr ? gamma : dst);
    auto src8_end = (const uint8_t *) src + length;

    while (src8 != src8_end) {
        *dst8 = *src8 ^ *gamma8;
        src8++;
        dst8++;
        gamma8++;
    }
}
