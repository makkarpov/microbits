#ifndef UB_TEST_CRYPTO_C25519_FPRIME_TEST_DATA_H
#define UB_TEST_CRYPTO_C25519_FPRIME_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct fp_load_test {
    uint32_t il;    //! Length of the input
    uint8_t  i[96]; //! Input number
    uint8_t  m[32]; //! Modulus
    uint8_t  r[32]; //! Expected result
};

struct fp_binary_test {
    uint8_t a[32];  //! Input number
    uint8_t b[32];  //! Input number
    uint8_t m[32];  //! Modulus
    uint8_t s[32];  //! (a + b) % m
    uint8_t p[32];  //! (a * b) % m
};

extern const fp_load_test * const fp_load_tests[];
extern const fp_binary_test * const fp_binary_tests[];

#endif // UB_TEST_CRYPTO_C25519_FPRIME_TEST_DATA_H
