#include "hash_test.hpp"

#include <ub/crypto/sha3.hpp>

#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace ub::crypto;

int main() {
    shake ctx;
    uint8_t digest[512];

    for (size_t i = 0; shake_128_samples[i] != nullptr; i++) {
        const hash_test_sample *t = shake_128_samples[i];

        ctx.reset(shake::FN_SHAKE128);
        ctx.update(shake_128_buffer, t->length);
        ctx.generate(digest, t->digestLen);

        if (std::memcmp(digest, t->digest, t->digestLen) != 0) {
            fprintf(stderr, "shake128 test failure on sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; shake_256_samples[i] != nullptr; i++) {
        const hash_test_sample *t = shake_256_samples[i];

        ctx.reset(shake::FN_SHAKE256);
        ctx.update(shake_256_buffer, t->length);
        ctx.generate(digest, t->digestLen);

        if (std::memcmp(digest, t->digest, t->digestLen) != 0) {
            fprintf(stderr, "shake256 test failure on sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}
