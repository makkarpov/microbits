#include "fprime8.hpp"

#include <algorithm>

using namespace ub::crypto::impl;

static void fp_raw_add(uint8_t *x, const uint8_t *p, size_t len) {
    uint32_t c = 0;
    for (size_t i = 0; i < len; i++) {
        c += (uint16_t) x[i] + (uint16_t) p[i];
        x[i] = c;
        c >>= 8;
    }
}

static void fp_select(uint8_t *r, bool cond, const uint8_t *v_false, const uint8_t *v_true, size_t len) {
    uint8_t mask = -(cond & 1);
    for (size_t i = 0; i < len; i++) {
        uint8_t diff = v_false[i] ^ v_true[i];
        r[i] = v_false[i] ^ (diff & mask);
    }
}

static void fp_raw_try_sub(uint8_t *x, const uint8_t *p, size_t len) {
    uint8_t minusP[len];

    uint32_t c = 0;
    for (size_t i = 0; i < len; i++) {
        c = (uint32_t) x[i] - (uint32_t) p[i] - c;
        minusP[i] = c;
        c = (c >> 8) & 1;
    }

    fp_select(x, c, minusP, x, len);
}

/* Warning: this function is variable-time */
static int fp_prime_msb(const uint8_t *p, size_t len) {
    uint8_t x;

    int i = (int) (len - 1);
    while (i >= 0 && p[i] == 0) i--;

    x = p[i];
    i <<= 3;

    while (x) {
        x >>= 1;
        i++;
    }

    return i - 1;
}

/* Warning: this function may be variable-time in the argument n */
static void fp_shift_n_bits(uint8_t *x, int n, size_t len) {
    uint32_t c = 0;
    for (size_t i = 0; i < len; i++) {
        c |= (uint32_t) x[i] << n;
        x[i] = c;
        c >>= 8;
    }
}

void Fp8::load(uint8_t *r, const uint8_t *data, size_t length, const fp8_field_t &f) {
    std::memset(r, 0, f.len);

    int preload_total = std::min(fp_prime_msb(f.mod, f.len) - 1, (int) (length << 3));
    int preload_bytes = preload_total >> 3;
    int preload_bits = preload_total & 7;
    int rbits = (int) (length << 3) - preload_total;

    for (int i = 0; i < preload_bytes; i++) {
        r[i] = data[length - preload_bytes + i];
    }

    if (preload_bits) {
        fp_shift_n_bits(r, preload_bits, f.len);
        r[0] |= data[length - preload_bytes - 1] >> (8 - preload_bits);
    }

    for (int i = rbits - 1; i >= 0; i--) {
        const uint8_t bit = (data[i >> 3] >> (i & 7)) & 1;

        fp_shift_n_bits(r, 1, f.len);
        r[0] |= bit;
        fp_raw_try_sub(r, f.mod, f.len);
    }
}

void Fp8::add(uint8_t *r, const uint8_t *a, const fp8_field_t &f) {
    fp_raw_add(r, a, f.len);
    fp_raw_try_sub(r, f.mod, f.len);
}

void Fp8::mul(uint8_t *r, const uint8_t *a, const uint8_t *b, const fp8_field_t &f) {
    uint8_t plusA[f.len];
    std::memset(r, 0, f.len);

    for (int32_t i = fp_prime_msb(f.mod, f.len); i >= 0; i--) {
        uint8_t bit = (b[i >> 3] >> (i & 7)) & 1;

        fp_shift_n_bits(r, 1, f.len);
        fp_raw_try_sub(r, f.mod, f.len);

        std::memcpy(plusA, r, f.len);
        add(plusA, a, f);

        fp_select(r, bit, r, plusA, f.len);
    }
}
