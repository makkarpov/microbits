#include "f448.hpp"

using namespace ub::crypto::impl;

enum { U32_224 = 7 };

// 32 bit words of little-endian P and 2P representation cast to (int8_t)
static const int8_t f448_modulus_i8[14]  = { -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1 };
static const int8_t f448_2modulus_i8[14] = { -2, -1, -1, -1, -1, -1, -1, -3, -1, -1, -1, -1, -1, -1 };

// both functions should compile to a single LDRSB instruction:

static inline uint32_t f448_modulus_word(size_t i) {
    return (uint32_t) (int32_t) f448_modulus_i8[i];
}

static inline uint32_t f448_2modulus_word(size_t i) {
    return (uint32_t) (int32_t) f448_2modulus_i8[i];
}

static uint32_t f448_add_carry(uint448_t &r, uint32_t carry, uint32_t i) {
    while (i < uint448_t::N_U32) {
        uint64_t rr = (uint64_t) r.u32[i] + carry;
        r.u32[i] = rr;
        carry = rr >> 32;
        i++;
    }

    return carry;
}

static uint32_t f448_add_buffers(uint32_t *r, const uint32_t *a, const uint32_t *b) {
    uint32_t carry = 0;

    for (size_t i = 0; i < uint448_t::N_U32; i++) {
        uint64_t rr = (uint64_t) carry + (uint64_t) a[i] + (uint32_t) b[i];
        r[i] = rr;
        carry = rr >> 32;
    }

    return carry;
}

static uint32_t f448_reduce(uint448_t &r, uint32_t carry) {
    // reduction is done by adding 2**224 + 1 for each carry bit
    // this is sufficient to keep resulting number correct modulo 2**448 - 2**224 - 1

    uint32_t rc = 0;
    rc |= f448_add_carry(r, carry, 0);
    rc |= f448_add_carry(r, carry, U32_224);
    return rc;
}

bool F448::normalize(uint448_t &r, const uint448_t &x) {
    uint448_t minusP;
    uint32_t borrow = 0;

    for (size_t i = 0; i < uint448_t::N_U32; i++) {
        uint64_t rr = (uint64_t) x.u32[i] - (uint64_t) f448_modulus_word(i) - borrow;
        minusP.u32[i] = rr;
        borrow = (rr >> 32) & 1;
    }

    bool overflow = (borrow & 1) ^ 1;
    r.select(overflow, x, minusP);
    return overflow;
}

void F448::load(uint448_t &r, int32_t x) {
    if (x < 0) {
        r.u32[0] = (uint32_t) (x - 1);

        for (size_t i = 1; i < uint448_t::N_U32; i++) {
            r.u32[i] = f448_modulus_word(i);
        }
    } else {
        r.u32[0] = x;
        std::memset(r.u32 + 1, 0, (uint448_t::N_U32 - 1) * sizeof(r.u32[0]));
    }
}

void F448::add(uint448_t &r, const uint448_t &a, const uint448_t &b) {
    uint32_t carry = f448_add_buffers(r.u32, a.u32, b.u32);
    f448_reduce(r, carry);
}

// Compute 2P - x, then reduce
void F448::neg(uint448_t &r, const uint448_t &x) {
    uint32_t borrow = 0;
    for (size_t i = 0; i < uint448_t::N_U32; i++) {
        uint64_t rr = (uint64_t) f448_2modulus_word(i) - (uint64_t) x.u32[i] - borrow;
        r.u32[i] = rr;
        borrow = (rr >> 32) & 1;
    }

    f448_reduce(r, 1 - borrow);
}

void F448::mul(uint448_t &r, const uint448_t &a, const uint448_t &b) {
    std::memset(r.u8, 0, uint448_t::N_U8);
    uint32_t tmp[2 * uint448_t::N_U32];

    // Step 1. Compute full product using normal word-by-word multiplication
    // Maximum possible carry length is 68 bits with only 36 bits preserved between iterations

    uint64_t c = 0;
    for (size_t i = 0; i < 2 * uint448_t::N_U32; i++) {
        uint32_t d = c;
        c = c >> 32;

        size_t start, end;
        if (i >= uint448_t::N_U32) {
            start = i - 13;
            end = uint448_t::N_U32;
        } else {
            start = 0;
            end = i + 1;
        }

        for (size_t j = start; j < end; j++) {
            uint64_t p = (uint64_t) a.u32[j] * (uint64_t) b.u32[i - j];
            uint64_t y = p + (uint64_t) d;

            d = y;
            c = c + (y >> 32);
        }

        tmp[i] = d;
    }

    // Step 2. Do two 'big' modular reductions, where length of carry could be 448 and 224 bits at most.
    for (size_t i = 0; i < 2; i++) {
        // Copy overflow value to `r` and zero it in `tmp`
        std::memcpy(r.u8, tmp + uint448_t::N_U32, uint448_t::N_U8);
        std::memset(tmp + uint448_t::N_U32, 0, uint448_t::N_U8);

        // Add overflow value to buffer:
        uint32_t carry = f448_add_buffers(tmp, tmp, r.u32);
        tmp[uint448_t::N_U32] = carry;

        // Add overflow value to buffer at 2**224 position:
        carry = f448_add_buffers(tmp + U32_224, tmp + U32_224, r.u32);
        tmp[uint448_t::N_U32 + U32_224] = carry;
    }

    // Step 3. Copy result back and do 'small' modular reductions, where length of carry is 1 bit at most
    std::memcpy(r.u8, tmp, uint448_t::N_U8);
    uint32_t carry = tmp[uint448_t::N_U32];
    carry = f448_reduce(r, carry);
    f448_reduce(r, carry);

    secureZero(tmp, sizeof(tmp));
}

void F448::inv(uint448_t &r, const uint448_t &x) {
    static const uint8_t powers[] = { 0xFF, 0xBF, 0x02, 0xFF, 0xBF, 0x02, 0x03, 0x00 };

    uint448_t s;
    bigint_pow_rle<uint448_t, F448>(r, s, x, powers);
}

void F448::powP34(uint448_t &r, const uint448_t &x) {
    static const uint8_t powers[] = { 0xFF, 0xBF, 0x02, 0xFF, 0xBF, 0x00 };

    uint448_t s;
    bigint_pow_rle<uint448_t, F448>(s, r, x, powers);
}
