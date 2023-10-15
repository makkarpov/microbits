#include "hash_test.hpp"
#include <ub/crypto/sha3.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace ub::crypto;

int main() {
    sha3 ctx;
    uint8_t digest[sha3::DIGEST_512];

    for (size_t i = 0; sha3_256_samples[i] != nullptr; i++) {
        const hash_test_sample *t = sha3_256_samples[i];

        ctx.reset(sha3::DIGEST_256);
        ctx.update(sha3_256_buffer, t->length);
        ctx.finish(digest);

        if (std::memcmp(digest, t->digest, sha3::DIGEST_256) != 0) {
            fprintf(stderr, "sha3_256 test failure at sample %zd\n", i);
            exit(1);
        }
    }

    for (size_t i = 0; sha3_512_samples[i] != nullptr; i++) {
        const hash_test_sample *t = sha3_512_samples[i];

        ctx.reset(sha3::DIGEST_512);
        ctx.update(sha3_512_buffer, t->length);
        ctx.finish(digest);

        if (std::memcmp(digest, t->digest, sha3::DIGEST_512) != 0) {
            fprintf(stderr, "sha3_512 test failure at sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}
