#include "mac_test_data.hpp"

#include <ub/crypto/sha2.hpp>
#include <ub/crypto/hmac.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace ub::crypto;

template <typename hash>
static void test(const mac_test * const * tests, const char *name) {
    hmac<hash> ctx;
    uint8_t digest[hash::OUTPUT];

    for (size_t i = 0; tests[i] != nullptr; i++) {
        const mac_test *t = tests[i];

        ctx.init(t->k, t->kl);
        ctx.update(t->data, t->len);
        ctx.finish(digest);

        if (std::memcmp(digest, t->m, hash::OUTPUT) != 0) {
            fprintf(stderr, "hmac<%s> test failed at sample %zd\n", name, i);
            exit(1);
        }
    }
}

int main() {
    test<sha256>(sha256_mac_tests, "sha256");
    test<sha512>(sha512_mac_tests, "sha512");
    return 0;
}