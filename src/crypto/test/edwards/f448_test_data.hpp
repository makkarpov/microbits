#ifndef UB_TEST_CRYPTO_EDWARDS_F448_TEST_DATA_H
#define UB_TEST_CRYPTO_EDWARDS_F448_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct f448_load_test {
    int32_t v;      //! Signed integer value to load
    uint8_t x[56];  //! Expected result
};

struct f448_unary_test {
    uint8_t x[56];  //! Input number
    uint8_t z[56];  //! `x % p`
    uint8_t n[56];  //! `p - x`
    uint8_t i[56];  //! `x^-1`
    uint8_t q[56];  //! `x^((p-3)//4)`
};

struct f448_binary_test {
    uint8_t x[56];  //! Input number
    uint8_t y[56];  //! Input number
    uint8_t s[56];  //! `x + y`
    uint8_t d[56];  //! `x - y`
    uint8_t e[56];  //! `y - x`
    uint8_t p[56];  //! `x * y`
};

extern const f448_load_test * const f448_load_tests[];
extern const f448_unary_test * const f448_unary_tests[];
extern const f448_binary_test * const f448_binary_tests[];

#endif // UB_TEST_CRYPTO_EDWARDS_F448_TEST_DATA_H
