#include "f25519.hpp"

#include <cstring>

using namespace ub::crypto::impl;

constexpr uint256_t ub::crypto::impl::C25519_ORDER {
    uint256_t::from_u8,
    0xED, 0xD3, 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, 0xD6, 0x9C, 0xF7, 0xA2, 0xDE, 0xF9, 0xDE, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
};

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
static void f25519_reduce_single(uint256_t &x, uint32_t carry) {
    // Reduce using 2^255 = 19 mod p
    uint32_t c = ((carry << 1) | (x.u32[7] >> 31)) * 19;
    x.u32[7] &= ~(1 << 31);

    u256_add_carry(x, x, uint256_t::N_U32, c);
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
    uint32_t c = 0;
    for (size_t i = 0; i < uint256_t::N_U8; i++) {
        size_t j = 0;

        for (; j <= i; j++) {
            c += (uint32_t) a.u8[j] * b.u8[i - j];
        }

        for (; j < uint256_t::N_U8; j++) {
            c += (uint32_t) a.u8[j] * b.u8[i + uint256_t::N_U8 - j] * 38;
        }

        r.u8[i] = c;
        c >>= 8;
    }

    f25519_reduce_single(r, c);
}

// carry has 32 bits, we process 8 bits at time, so b must be 24 bits at most
void F25519::mul_u24(uint256_t &r, const uint256_t &a, uint32_t b) {
    uint32_t c = 0;

    for (size_t i = 0; i < uint256_t::N_U8; i++) {
        c += b * (uint32_t) a.u8[i];
        r.u8[i] = c;
        c >>= 8;
    }

    f25519_reduce_single(r, c);
}

// interpret `powers` array as a sequence of run-length encoded bits of power, starting from most significant bit.
// Lowest bit of opcode encodes the actual power bit, higher bits encode repetition count.
static void f25519_pow_rle(uint256_t &r, uint256_t &s, const uint256_t &x, const uint8_t *powers) {
    uint256_t *a = &s;
    uint256_t *b = &r;

    std::memcpy(r.u8, x.u8, sizeof(r.u8));

    uint8_t i;
    while ((i = *powers) != 0) {
        while (i > 1) {
            F25519::mul(*a, *b, *b);

            if (i & 1) {
                F25519::mul(*b, *a, x);
            } else {
                std::swap(a, b);
            }

            i -= 2;
        }

        powers++;
    }
}

void F25519::inv(uint256_t &r, const uint256_t &x) {
    // Power opcodes for (q-2) = 2^255-21
    static uint8_t powers[] = { 255, 245, 2, 3, 2, 5, 0 };

    uint256_t s;
    f25519_pow_rle(r, s, x, powers);

    s.destroy();
}

void F25519::sqrt(uint256_t &r, const uint256_t &a) {
    // Powers opcodes for (q-5)/8 = 2^252-3
    static uint8_t powers2523[] = { 255, 245, 2, 3, 0 };

    using namespace F25519;
    uint256_t v, i, x, y;

    mul_u24(x, a, 2);
    f25519_pow_rle(y, v, x, powers2523);

    mul(y, v, v);
    mul(i, x, y);

    y = 1;
    sub(i, i, y);

    mul(x, v, a);
    mul(r, x, i);

    v.destroy();
    i.destroy();
    x.destroy();
}
