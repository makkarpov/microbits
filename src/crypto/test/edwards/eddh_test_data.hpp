#ifndef UB_TEST_CRYPTO_EDWARDS_EDDH_TEST_DATA_H
#define UB_TEST_CRYPTO_EDWARDS_EDDH_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct eddh_public_test {
    uint8_t prv[56];    //! Private key
    uint8_t pub[56];    //! Expected public key
};

struct eddh_compute_test {
    uint8_t prv[56];    //! Private key
    uint8_t pub[56];    //! Public key
    uint8_t sec[56];    //! Expected shared secret
};

extern const eddh_public_test * const x25519_public_tests[];
extern const eddh_compute_test * const x25519_compute_tests[];
extern const eddh_public_test * const x448_public_tests[];
extern const eddh_compute_test * const x448_compute_tests[];

#endif // UB_TEST_CRYPTO_EDWARDS_EDDH_TEST_DATA_H
