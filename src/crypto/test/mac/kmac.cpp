#include <ub/crypto/kmac.hpp>

#include "mac_test_data.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace ub::crypto;

static void test(uint32_t variant, const mac_test * const * tests, const char *name) {
    kmac ctx;

    for (size_t i = 0; tests[i] != nullptr; i++) {
        const mac_test *t = tests[i];

        ctx.init(variant, t->k, t->kl, t->ml);
        ctx.update(t->data, t->len);

        uint8_t result[t->ml];
        ctx.finish(result);

        if (std::memcmp(t->m, result, t->ml) != 0) {
            fprintf(stderr, "%s test failed at sample %zd\n", name, i);
            exit(1);
        }
    }
}

int main() {
    test(kmac::KMAC_128, kmac128_mac_tests, "kmac128");
    test(kmac::KMAC_256, kmac256_mac_tests, "kmac256");
    return 0;
}
