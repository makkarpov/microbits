#ifndef UB_TEST_CRYPTO_C25519_F25519_TEST_DATA_H
#define UB_TEST_CRYPTO_C25519_F25519_TEST_DATA_H

#include <cstdint>
#include <cstddef>

#include <c25519/f25519.hpp>

/** Number normalization test samples */
struct f25519_norm_test {
    uint8_t x[32];  //! Input
    uint8_t y[32];  //! `x mod p`
};

struct f25519_unary_test {
    uint8_t x[32];  //! Input
    uint8_t n[32];  //! `-x`
    uint8_t i[32];  //! `x^(-1) mod p`
    bool    re;     //! Whether square root of `x` exists
    uint8_t r[32];  //! `sqrt(x)`
};

struct f25519_mul_u32_test {
    uint8_t  x[32]; //! First input
    uint32_t y;     //! Second input
    uint8_t  p[32]; //! `x * y`
};

struct f25519_binary_test {
    uint8_t x[32];  //! First input
    uint8_t y[32];  //! Second input
    uint8_t s[32];  //! `x + y`
    uint8_t d[32];  //! `x - y`
    uint8_t e[32];  //! `y - x`
    uint8_t p[32];  //! `x * y`
};

extern const f25519_norm_test * const f25519_norm_tests[];
extern const f25519_unary_test * const f25519_unary_tests[];
extern const f25519_mul_u32_test * const f25519_mul_u32_tests[];
extern const f25519_binary_test * const f25519_binary_tests[];

#endif // UB_TEST_CRYPTO_C25519_F25519_TEST_DATA_H
