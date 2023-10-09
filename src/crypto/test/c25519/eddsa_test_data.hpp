#ifndef UB_TEST_CRYPTO_C25519_EDDSA_TEST_DATA_H
#define UB_TEST_CRYPTO_C25519_EDDSA_TEST_DATA_H

#include <cstddef>
#include <cstdint>

enum {
    MSG_LEN_ED25519ph   = 0xFFFFFF
};

struct eddsa_public_key_test {
    uint8_t x[32];  //! Private key
    uint8_t y[32];  //! Expected public key
};

struct eddsa_sign_test {
    uint8_t key[32];    //! Private key
    size_t  len;        //! Message length and selector for Ed25519ph mode
    uint8_t msg[256];   //! Message
    uint8_t sig[64];    //! Valid signature
};

struct eddsa_verify_test {
    uint8_t key[32];    //! Public key
    size_t  len;        //! Message length and selector for Ed25519ph mode
    uint8_t msg[256];   //! Message
    uint8_t sig[64];    //! Signature
    bool    valid;      //! Whether verification should succeed
};

extern const eddsa_public_key_test * const eddsa_public_key_tests[];
extern const eddsa_sign_test * const eddsa_sign_tests[];
extern const eddsa_verify_test * const eddsa_verify_tests[];

#endif // UB_TEST_CRYPTO_C25519_EDDSA_TEST_DATA_H
