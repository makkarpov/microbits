#ifndef UB_TEST_CRYPTO_MAC_TEST_DATA_H
#define UB_TEST_CRYPTO_MAC_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct mac_test {
    const uint8_t *data;    //! Data buffer
    size_t  len;            //! Length of data to include in MAC
    size_t  kl;             //! Key length
    uint8_t k[256];         //! Key data
    size_t  ml;             //! MAC length
    uint8_t m[128];         //! Expected MAC
};

extern const mac_test * const sha256_mac_tests[];
extern const mac_test * const sha512_mac_tests[];
extern const mac_test * const kmac128_mac_tests[];
extern const mac_test * const kmac256_mac_tests[];

#endif // UB_TEST_CRYPTO_MAC_TEST_DATA_H
