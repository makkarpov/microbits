#ifndef UB_TEST_CRYPTO_CIPHER_STREAM_TEST_DATA_H
#define UB_TEST_CRYPTO_CIPHER_STREAM_TEST_DATA_H

#include <cstdint>
#include <cstddef>

struct cipher_stream_test {
    size_t  kl;     //! Key length in bytes
    uint8_t k[32];  //! Key data
    size_t  nl;     //! Nonce length in bytes
    uint8_t n[32];  //! Nonce data
    size_t  sl;     //! Key stream length in bytes
    uint8_t s[256]; //! Expected key stream
};

extern const cipher_stream_test * const chacha20_tests[];

#endif // UB_TEST_CRYPTO_CIPHER_STREAM_TEST_DATA_H
