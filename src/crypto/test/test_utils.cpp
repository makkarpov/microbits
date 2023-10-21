#include "test_utils.hpp"

#include <cstdio>

#include <edwards/bigint.hpp>

using namespace ub::crypto::impl;

static void printNumber(const char *prefix, const uint8_t *x, size_t len) {
    fprintf(stderr, "%s: ", prefix);
    size_t i = len;
    do {
        i--;
        fprintf(stderr, "%02X", x[i]);
    } while (i != 0);
    fprintf(stderr, "\n");
}

void printNumber256(const char *prefix, const uint8_t *x) {
    printNumber(prefix, x, uint256_t::N_U8);
}

void printNumber448(const char *prefix, const uint8_t *x) {
    printNumber(prefix, x, uint448_t::N_U8);
}

void printBytes(const char *prefix, const uint8_t *x, size_t length) {
    fprintf(stderr, "%s: ", prefix);
    for (size_t i = 0; i < length; i++) fprintf(stderr, "%02X", x[i]);
    fprintf(stderr, "\n");
}
