#ifndef UB_TEST_CRYPTO_EDWARDS_EDDSA_TEST_DATA_H
#define UB_TEST_CRYPTO_EDWARDS_EDDSA_TEST_DATA_H

#include <cstddef>
#include <cstdint>

enum {
    MSG_LEN_PREHASH  = 0xFFFFFF
};

struct eddsa_public_key_test {
    uint8_t x[57];      //! Private key
    uint8_t y[57];      //! Expected public key
};

struct eddsa_sign_test {
    uint8_t key[114];   //! Private key + public key
    size_t  len;        //! Message length and selector for Ed25519ph mode
    uint8_t msg[256];   //! Message
    uint8_t sig[114];   //! Valid signature
};

struct eddsa_verify_test {
    uint8_t key[57];    //! Public key
    size_t  len;        //! Message length and selector for Ed25519ph mode
    uint8_t msg[256];   //! Message
    uint8_t sig[114];   //! Signature
    bool    valid;      //! Whether verification should succeed
};

extern const eddsa_public_key_test * const eddsa25519_public_key_tests[];
extern const eddsa_sign_test * const eddsa25519_sign_tests[];
extern const eddsa_verify_test * const eddsa25519_verify_tests[];
extern const eddsa_public_key_test * const eddsa448_public_key_tests[];
extern const eddsa_sign_test * const eddsa448_sign_tests[];
extern const eddsa_verify_test * const eddsa448_verify_tests[];

#endif // UB_TEST_CRYPTO_EDWARDS_EDDSA_TEST_DATA_H
