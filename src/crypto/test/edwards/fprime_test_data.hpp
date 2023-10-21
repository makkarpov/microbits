#ifndef UB_TEST_CRYPTO_EDWARDS_FPRIME_TEST_DATA_H
#define UB_TEST_CRYPTO_EDWARDS_FPRIME_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct fp_load_test {
    uint32_t ml;    //! Length of the modulus
    uint32_t il;    //! Length of the input
    uint8_t  i[96]; //! Input number
    uint8_t  m[64]; //! Modulus
    uint8_t  r[64]; //! Expected result
};

struct fp_binary_test {
    uint32_t ml;    //! Length of the modulus
    uint8_t  a[64]; //! Input number
    uint8_t  b[64]; //! Input number
    uint8_t  m[64]; //! Modulus
    uint8_t  s[64]; //! (a + b) % m
    uint8_t  p[64]; //! (a * b) % m
};

extern const fp_load_test * const fp_load_tests[];
extern const fp_binary_test * const fp_binary_tests[];

#endif // UB_TEST_CRYPTO_EDWARDS_FPRIME_TEST_DATA_H
