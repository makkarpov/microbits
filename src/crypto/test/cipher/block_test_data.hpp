#ifndef UB_TEST_CRYPTO_CIPHER_BLOCK_TEST_DATA_H
#define UB_TEST_CRYPTO_CIPHER_BLOCK_TEST_DATA_H

#include <cstddef>
#include <cstdint>

struct cipher_block_test {
    bool    enc;    //! true - encryption, false - decryption
    size_t  kl;     //! Length of the key in bytes
    uint8_t k[32];  //! Encryption key
    uint8_t i[32];  //! Input block
    uint8_t o[32];  //! Expected output block
};

extern const cipher_block_test * const aes_block_tests[];

#endif // UB_TEST_CRYPTO_CIPHER_BLOCK_TEST_DATA_H
