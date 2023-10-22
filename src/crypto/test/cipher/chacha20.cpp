#include "stream_test_data.hpp"

#include <ub/crypto/chacha20.hpp>

#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace ub::crypto;

int main() {
    for (size_t i = 0; chacha20_tests[i] != nullptr; i++) {
        const cipher_stream_test *t = chacha20_tests[i];

        chacha20 ctx;
        ctx.init(t->k, t->n);

        uint8_t output[t->sl];
        std::memset(output, 0, t->sl);
        ctx.process(output, output, t->sl);

        if (std::memcmp(output, t->s, t->sl) != 0) {
            fprintf(stderr, "chacha20 test failed at sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}