#include "eddsa_test_data.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <test_utils.hpp>
#include <ub/crypto/edwards.hpp>

using namespace ub::crypto;

int main() {
    for (size_t i = 0; eddsa25519_public_key_tests[i] != nullptr; i++) {
        const eddsa_public_key_test *t = eddsa25519_public_key_tests[i];

        uint8_t publicKey[ed25519::KEY_LENGTH];
        ed25519::toPublic(publicKey, t->x);

        if (std::memcmp(publicKey, t->y, ed25519::KEY_LENGTH) != 0) {
            fprintf(stderr, "ed25519::toPublic test failed at sample %zd\n", i);
            printBytes("Expected", t->y, ed25519::KEY_LENGTH);
            printBytes("Actual  ", publicKey, ed25519::KEY_LENGTH);
            exit(1);
        }
    }

    for (size_t i = 0; eddsa25519_sign_tests[i] != nullptr; i++) {
        const eddsa_sign_test *t = eddsa25519_sign_tests[i];

        uint8_t signature[ed25519::SIGNATURE_LENGTH];

        if (t->len == MSG_LEN_PREHASH) {
            ed25519::signHash(t->key, signature, t->msg);
        } else {
            ed25519::sign(t->key, signature, t->msg, t->len);
        }

        if (std::memcmp(signature, t->sig, ed25519::SIGNATURE_LENGTH) != 0) {
            fprintf(stderr, "ed25519::sign test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; eddsa25519_verify_tests[i] != nullptr; i++) {
        const eddsa_verify_test *t = eddsa25519_verify_tests[i];

        bool valid;

        if (t->len == MSG_LEN_PREHASH) {
            valid = ed25519::verifyHash(t->key, t->sig, t->msg);
        } else {
            valid = ed25519::verify(t->key, t->sig, t->msg, t->len);
        }

        if (valid != t->valid) {
            fprintf(stderr, "ed25519::verify test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; eddsa448_public_key_tests[i] != nullptr; i++) {
        const eddsa_public_key_test *t = eddsa448_public_key_tests[i];

        uint8_t publicKey[ed448::KEY_LENGTH];
        ed448::toPublic(publicKey, t->x);

        if (std::memcmp(publicKey, t->y, ed448::KEY_LENGTH) != 0) {
            fprintf(stderr, "ed448::toPublic test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; eddsa448_sign_tests[i] != nullptr; i++) {
        const eddsa_sign_test *t = eddsa448_sign_tests[i];
        uint8_t signature[ed448::SIGNATURE_LENGTH];

        if (t->len == MSG_LEN_PREHASH) {
            ed448::signHash(t->key, signature, t->msg);
        } else {
            ed448::sign(t->key, signature, t->msg, t->len);
        }

        if (std::memcmp(signature, t->sig, ed448::SIGNATURE_LENGTH) != 0) {
            fprintf(stderr, "ed448::sign test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; eddsa448_verify_tests[i] != nullptr; i++) {
        const eddsa_verify_test *t = eddsa448_verify_tests[i];

        bool valid;

        if (t->len == MSG_LEN_PREHASH) {
            valid = ed448::verifyHash(t->key, t->sig, t->msg);
        } else {
            valid = ed448::verify(t->key, t->sig, t->msg, t->len);
        }

        if (valid != t->valid) {
            fprintf(stderr, "ed448::verify test failed at sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}