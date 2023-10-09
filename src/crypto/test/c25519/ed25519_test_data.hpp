#ifndef UB_TEST_CRYPTO_C25519_ED25519_TEST_DATA_H
#define UB_TEST_CRYPTO_C25519_ED25519_TEST_DATA_H

#include <cstdint>

struct ed25519_add_test {
    uint8_t x1[32]; //! X coord of first point
    uint8_t y1[32]; //! Y coord of first point
    uint8_t x2[32]; //! X coord of second point
    uint8_t y2[32]; //! Y coord of second point
    uint8_t xr[32]; //! X coord of sum
    uint8_t yr[32]; //! Y coord of sum
};

struct ed25519_mul_test {
    uint8_t x1[32]; //! X coord of first point
    uint8_t y1[32]; //! Y coord of first point
    uint8_t k[32];  //! Scalar multiplier
    uint8_t xr[32]; //! X coord of product
    uint8_t yr[32]; //! Y coord of product
};

struct ed25519_load_test {
    uint8_t b[32];  //! Packed point
    bool valid;     //! Whether packed point is valid
    uint8_t xr[32]; //! X coord of the result
    uint8_t yr[32]; //! Y coord of the result
};

extern const ed25519_add_test * const ed25519_add_tests[];
extern const ed25519_mul_test * const ed25519_mul_tests[];
extern const ed25519_load_test * const ed25519_load_tests[];

#endif // UB_TEST_CRYPTO_C25519_ED25519_TEST_DATA_H
