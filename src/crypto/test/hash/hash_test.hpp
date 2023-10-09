#ifndef UB_TEST_CRYPTO_HASH_TEST_H
#define UB_TEST_CRYPTO_HASH_TEST_H

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>

struct hash_test_sample {
    size_t        length;
    const uint8_t *digest;
};

extern const uint8_t sha256_buffer[];
extern const hash_test_sample * const sha256_samples[];

extern const uint8_t sha512_buffer[];
extern const hash_test_sample * const sha512_samples[];

template <typename hash>
int runHashTests(const char *name, const uint8_t *buffer, const hash_test_sample * const * samples) {
    hash ctx;
    uint8_t digest[hash::OUTPUT];

    size_t i = 0;
    for (; sha256_samples[i] != nullptr; i++) {
        const hash_test_sample *s = samples[i];

        ctx.reset();
        ctx.update(buffer, s->length);
        ctx.finish(digest);

        if (memcmp(digest, s->digest, hash::OUTPUT) != 0) {
            fprintf(stderr, "%s test failure on length %zd\n", name, s->length);
            return 1;
        }
    }

    printf("%s test ok: %zd samples\n", name, i);
    return 0;
}

#endif // UB_TEST_CRYPTO_HASH_TEST_H
