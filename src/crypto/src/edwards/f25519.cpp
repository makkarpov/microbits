#include "f25519.hpp"

#include <cstring>

using namespace ub::crypto::impl;

static uint32_t u256_add_carry(const uint256_t &src, uint256_t &dst, size_t length, uint32_t carry) {
    uint64_t carry64 = carry;

    for (size_t i = 0; i < length; i++) {
        carry64 += src.u32[i];
        dst.u32[i] = carry64;
        carry64 >>= 32;
    }

    return (uint32_t) carry64;
}

// required carry bitness: add: 1, sub: ?, mul: ?
// mul 8b: 14, 16b: 22, 32b: 38
static void f25519_reduce_single(uint256_t &x, uint64_t carry) {
    // Reduce using 2^255 = 19 mod p
    carry <<= 1;
    carry |= x.u32[7] >> 31;
    carry *= 19;

    x.u32[7] &= ~(1 << 31);

    for (uint32_t &i: x.u32) {
        carry += i;
        i = carry;
        carry >>= 32;
    }
}

void F25519::normalize(uint256_t &x) {
    f25519_reduce_single(x, 0);

    // The number is now less than 2^255 + 18, and therefore less than
    // 2p. Try subtracting p, and conditionally load the subtracted
    // value if underflow did not occur.

    uint256_t minusP;
    uint32_t c = u256_add_carry(x, minusP, uint256_t::N_U32 - 1, 19);

    c += x.u32[7] - 0x80000000;
    minusP.u32[7] = c;

    x.select((c >> 31) & 1, minusP, x);
    minusP.destroy();
}

void F25519::add(uint256_t &r, const uint256_t &a, const uint256_t &b) {
    uint32_t carry = 0;

    for (size_t i = 0; i < uint256_t::N_U32; i++) {
        uint64_t result = (uint64_t) a.u32[i] + (uint64_t) b.u32[i] + carry;
        r.u32[i] = result;
        carry = result >> 32;
    }

    f25519_reduce_single(r, carry);
}

void F25519::sub(uint256_t &r, const uint256_t &a, const uint256_t &b) {
    /* Calculate a + 2p - b, to avoid underflow */
    uint32_t c = 0xDA;
    for (size_t i = 0; i < uint256_t::N_U8 - 1; i++) {
        c += 0xFF00 + (uint32_t) a.u8[i] - (uint32_t) b.u8[i];
        r.u8[i] = c;
        c >>= 8;
    }

    c += (uint32_t) a.u8[31] - (uint32_t) b.u8[31];
    r.u8[31] = c;
    c >>= 8;

    f25519_reduce_single(r, c);
}

void F25519::neg(uint256_t &r, const uint256_t &x) {
    uint32_t c = 0xDA;
    for (size_t i = 0; i < uint256_t::N_U8 - 1; i++) {
        c += 0xFF00 - (uint32_t) x.u8[i];
        r.u8[i] = c;
        c >>= 8;
    }

    c -= (uint32_t) x.u8[31];
    r.u8[31] = c;
    c >>= 8;

    f25519_reduce_single(r, c);
}

void F25519::mul(uint256_t &r, const uint256_t &a, const uint256_t &b) {
    // Carry is stored as 96-bit number where c1 stores upper 64 bits and c0 stores lower 32 bits.
    // Only 64 bits are necessary between loop iterations once lower 32 bits are shifted to result
    uint64_t c1 = 0;

    for (size_t i = 0; i < uint256_t::N_U32; i++) {
        size_t j = 0;

        uint32_t c0 = c1;
        c1 >>= 32;

        for (; j <= i; j++) {
            uint64_t x = (uint64_t) a.u32[j] * b.u32[i - j];
            uint64_t y = (uint64_t) c0 + (uint32_t) x;

            c0 = (uint32_t) y;
            c1 = c1 + (uint64_t) (x >> 32) + (uint64_t) (y >> 32);
        }

        for (; j < uint256_t::N_U32; j++) {
            uint64_t x = (uint64_t) a.u32[j] * (uint64_t) b.u32[i + uint256_t::N_U32 - j];
            uint64_t y = (uint64_t) c0 + (uint64_t) ((uint32_t) x) * 38;

            c0 = (uint32_t) y;
            c1 = c1 + ((x >> 32) * 38) + (y >> 32);
        }

        r.u32[i] = c0;
    }

    f25519_reduce_single(r, c1);
}

void F25519::inv(uint256_t &r, const uint256_t &x) {
    // Power opcodes for (q-2) = 2^255-21
    static uint8_t powers[] = { 255, 245, 2, 3, 2, 5, 0 };

    uint256_t s;
    bigint_pow_rle<uint256_t, F25519>(r, s, x, powers);

    s.destroy();
}

void F25519::pow58(uint256_t &r, const uint256_t &x) {
    // Powers opcodes for (q-5)/8 = 2^252-3
    static uint8_t powers2523[] = { 255, 245, 2, 3, 0 };

    uint256_t t;
    bigint_pow_rle<uint256_t, F25519>(t, r, x, powers2523);

    t.destroy();
}
