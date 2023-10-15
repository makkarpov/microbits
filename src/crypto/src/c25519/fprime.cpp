#include "fprime.hpp"

#include <algorithm>

using namespace ub::crypto::impl;

static void fp_raw_add(uint256_t &x, const uint256_t &p) {
    uint16_t c = 0;

    for (size_t i = 0; i < uint256_t::N_U8; i++) {
        c += (uint16_t) x.u8[i] + (uint16_t) p.u8[i];
        x.u8[i] = c;
        c >>= 8;
    }
}

static void fp_raw_try_sub(uint256_t &x, const uint256_t &p) {
    uint256_t minusP;
    uint32_t c = 0;

    for (size_t i = 0; i < uint256_t::N_U16; i++) {
        c = (uint32_t) x.u16[i] - (uint32_t) p.u16[i] - c;
        minusP.u16[i] = c;
        c = (c >> 16) & 1;
    }

    x.select(c, minusP, x);
}

/* Warning: this function is variable-time */
static int fp_prime_msb(const uint256_t &p) {
    uint8_t x;

    int i = uint256_t::N_U8 - 1;
    while (i >= 0 && p.u8[i] == 0) i--;

    x = p.u8[i];
    i <<= 3;

    while (x) {
        x >>= 1;
        i++;
    }

    return i - 1;
}

/* Warning: this function may be variable-time in the argument n */
static void fp_shift_n_bits(uint256_t &x, int n) {
    uint32_t c = 0;

    for (size_t i = 0; i < uint256_t::N_U16; i++) {
        c |= (uint32_t) x.u16[i] << n;
        x.u16[i] = c;
        c >>= 16;
    }
}

void Fp::load(uint256_t &r, const uint8_t *data, size_t length, const uint256_t &m) {
    std::memset(r.u8, 0, sizeof(r.u8));

    int preload_total = std::min(fp_prime_msb(m) - 1, (int) (length << 3));
    int preload_bytes = preload_total >> 3;
    int preload_bits = preload_total & 7;
    int rbits = (int) (length << 3) - preload_total;

    for (int i = 0; i < preload_bytes; i++) {
        r.u8[i] = data[length - preload_bytes + i];
    }

    if (preload_bits) {
        fp_shift_n_bits(r, preload_bits);
        r.u8[0] |= data[length - preload_bytes - 1] >> (8 - preload_bits);
    }

    for (int i = rbits - 1; i >= 0; i--) {
        const uint8_t bit = (data[i >> 3] >> (i & 7)) & 1;

        fp_shift_n_bits(r, 1);
        r.u8[0] |= bit;
        fp_raw_try_sub(r, m);
    }
}

void Fp::add(uint256_t &r, const uint256_t &a, const uint256_t &m) {
    fp_raw_add(r, a);
    fp_raw_try_sub(r, m);
}

void Fp::mul(uint256_t &r, const uint256_t &a, const uint256_t &b, const uint256_t &m) {
    uint256_t plusA;
    std::memset(r.u8, 0, uint256_t::N_U8);

    for (int32_t i = fp_prime_msb(m); i >= 0; i--) {
        uint8_t bit = (b.u8[i >> 3] >> (i & 7)) & 1;

        fp_shift_n_bits(r, 1);
        fp_raw_try_sub(r, m);

        std::memcpy(plusA.u8, r.u8, uint256_t::N_U8);
        Fp::add(plusA, a, m);

        r.select(bit, r, plusA);
    }
}
