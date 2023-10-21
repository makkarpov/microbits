#include <ub/crypto/edwards.hpp>
#include <test_utils.hpp>

#include "eddh_test_data.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace ub::crypto;

int main() {
    for (size_t i = 0; x25519_public_tests[i] != nullptr; i++) {
        const eddh_public_test *t = x25519_public_tests[i];

        uint8_t publicKey[x25519::LENGTH];
        x25519::toPublic(publicKey, t->prv);

        if (std::memcmp(publicKey, t->pub, x25519::LENGTH) != 0) {
            fprintf(stderr, "x25519::toPublic test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; x25519_compute_tests[i] != nullptr; i++) {
        const eddh_compute_test *t = x25519_compute_tests[i];

        uint8_t secret[x25519::LENGTH];
        x25519::compute(secret, t->prv, t->pub);

        if (std::memcmp(secret, t->sec, x25519::LENGTH) != 0) {
            fprintf(stderr, "x25519::compute test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; x448_public_tests[i] != nullptr; i++) {
        const eddh_public_test *t = x448_public_tests[i];

        uint8_t publicKey[x448::LENGTH];
        x448::toPublic(publicKey, t->prv);

        if (std::memcmp(publicKey, t->pub, x448::LENGTH) != 0) {
            fprintf(stderr, "x448::toPublic test failed at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; x448_compute_tests[i] != nullptr; i++) {
        const eddh_compute_test *t = x448_compute_tests[i];

        uint8_t secret[x448::LENGTH];
        x448::compute(secret, t->prv, t->pub);

        if (std::memcmp(secret, t->sec, x448::LENGTH) != 0) {
            fprintf(stderr, "x448::compute test failed at sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}