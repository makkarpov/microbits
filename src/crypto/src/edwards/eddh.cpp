#include <ub/crypto/edwards.hpp>

#include <cstring>

#include "bigint.hpp"
#include "f25519.hpp"
#include "f448.hpp"

using namespace ub::crypto;
using namespace ub::crypto::impl;

constexpr static uint32_t X25519_K      = 121665;
constexpr static uint32_t X25519_U      = 9;
constexpr static uint32_t X25519_BITS   = 255;
constexpr static uint32_t X448_K        = 39081;
constexpr static uint32_t X448_U        = 5;
constexpr static uint32_t X448_BITS     = 448;

template <typename F, uint32_t K_a24, uint32_t N_bits, typename uintX_t = typename F::uint_t>
static void edwards_multiply(uint8_t *r, const uintX_t &k, const uintX_t &u) {
    uintX_t x2, z2, x3, z3;
    uintX_t t[4];

    x2 = 1;
    x3 = u;
    z3 = 1;

    size_t i = N_bits;
    bool swap = false;

    while (i != 0) {
        i--;
        bool bit = (k.u8[i >> 3] >> (i & 7)) & 1;

        swap ^= bit;
        uintX_t::swap(swap, x2, x3);
        uintX_t::swap(swap, z2, z3);
        swap = bit;

        F::add(t[0], x2, z2);       // t[0]: A = x2 + z2
        F::neg(z2, z2);
        F::add(t[1], x2, z2);       // t[1]: B = x2 - z2

        F::add(t[2], x3, z3);       // t[2]: C = x3 + z3
        F::neg(z3, z3);
        F::add(t[3], x3, z3);       // t[3]: D = x3 - z3

        F::mul(x2, t[3], t[0]);     // x2: DA = D * A
        F::mul(z2, t[2], t[1]);     // z2: CB = C * B

        F::add(t[2], x2, z2);       // t[2]: DA + CB
        F::neg(t[3], z2);           // t[3]: -CB

        F::mul(x3, t[2], t[2]);     // x3 = (DA + CB)^2

        F::add(t[3], x2, t[3]);     // t[3]: DA - CB
        F::mul(t[2], t[3], t[3]);   // t[2]: (DA - CB)^2
        F::mul(z3, t[2], u);        // z3 = u * (DA - CB)^2

        F::mul(t[2], t[0], t[0]);   // t[2]: AA = A^2
        F::mul(t[3], t[1], t[1]);   // t[3]: BB = B^2
        F::mul(x2, t[2], t[3]);     // x2 = AA * BB

        F::neg(t[3], t[3]);
        F::add(t[0], t[2], t[3]);   // t[0]: E = AA - BB

        t[1] = K_a24;
        F::mul(z2, t[1], t[0]);     // z2: a24 * E
        F::add(t[2], t[2], z2);     // t[2]: AA + a24 * E
        F::mul(z2, t[2], t[0]);     // z2 = E * (AA + a24 * E)
    }

    uintX_t::swap(swap, x2, x3);
    uintX_t::swap(swap, z2, z3);

    F::inv(t[0], z2);
    F::mul(t[1], x2, t[0]);
    F::normalize(t[1]);

    std::memcpy(r, t[1].u8, uintX_t::N_U8);

    ub::crypto::secureZero(t, sizeof(t));
}

static inline void x25519_multiply(uint8_t *r, const uint256_t &k, const uint256_t &u) {
    edwards_multiply<F25519, X25519_K, X25519_BITS>(r, k, u);
}

static inline void x448_multiply(uint8_t *r, const uint448_t &k, const uint448_t &u) {
    edwards_multiply<F448, X448_K, X448_BITS>(r, k, u);
}

static void x25519_load_scalar(uint256_t &r, const uint8_t *src) {
    std::memcpy(r.u8, src, uint256_t::N_U8);
    r.u8[0] &= 0xF8;
    r.u8[31] = (r.u8[31] & 0x3F) | 0x40;
}

static void x448_load_scalar(uint448_t &r, const uint8_t *src) {
    std::memcpy(r.u8, src, uint448_t::N_U8);
    r.u8[0] &= 0xFC;
    r.u8[55] |= 0x80;
}

void x25519::toPublic(uint8_t *publicKey, const uint8_t *privateKey) {
    uint256_t k, u;

    u = X25519_U;
    x25519_load_scalar(k, privateKey);

    x25519_multiply(publicKey, k, u);

    k.destroy();
}

void x25519::compute(uint8_t *secret, const uint8_t *privateKey, const uint8_t *publicKey) {
    uint256_t k, u;

    std::memcpy(u.u8, publicKey, uint256_t::N_U8);
    F25519::normalize(u);

    x25519_load_scalar(k, privateKey);

    x25519_multiply(secret, k, u);

    k.destroy();
}

void x448::toPublic(uint8_t *publicKey, const uint8_t *privateKey) {
    uint448_t k, u;

    u = X448_U;
    x448_load_scalar(k, privateKey);

    x448_multiply(publicKey, k, u);

    k.destroy();
}

void x448::compute(uint8_t *secret, const uint8_t *privateKey, const uint8_t *publicKey) {
    uint448_t k, u;

    std::memcpy(u.u8, publicKey, uint448_t::N_U8);
    F448::normalize(u, u);

    x448_load_scalar(k, privateKey);
    x448_multiply(secret, k, u);

    k.destroy();
}
