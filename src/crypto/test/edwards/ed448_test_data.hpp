#ifndef UB_TEST_CRYPTO_EDWARDS_ED448_TEST_DATA_H
#define UB_TEST_CRYPTO_EDWARDS_ED448_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct ed448_add_test {
    uint8_t x1[56];
    uint8_t y1[56];
    uint8_t x2[56];
    uint8_t y2[56];
    uint8_t xr[56];
    uint8_t yr[56];
};

struct ed448_mul_test {
    uint8_t xi[56];
    uint8_t yi[56];
    uint8_t k[56];
    uint8_t xr[56];
    uint8_t yr[56];
};

struct ed448_load_test {
    uint8_t x[56];
    uint8_t y[56];
    bool    valid;
    uint8_t b[57];
};

extern const ed448_add_test * const ed448_add_tests[];
extern const ed448_mul_test * const ed448_mul_tests[];
extern const ed448_load_test * const ed448_load_tests[];

#endif // UB_TEST_CRYPTO_EDWARDS_ED448_TEST_DATA_H
