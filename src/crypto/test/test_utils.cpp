#include "test_utils.hpp"

#include <cstdio>

#include <c25519/uint256.hpp>

using namespace ub::crypto::impl;

void printNumber(const char *prefix, const uint8_t *x) {
    fprintf(stderr, "%s: ", prefix);
    size_t i = uint256_t::N_U8;
    do {
        i--;
        fprintf(stderr, "%02X", x[i]);
    } while (i != 0);
    fprintf(stderr, "\n");
}

void printBytes(const char *prefix, const uint8_t *x, size_t length) {
    fprintf(stderr, "%s: ", prefix);
    for (size_t i = 0; i < length; i++) printf("%02X", x[i]);
    fprintf(stderr, "\n");
}
