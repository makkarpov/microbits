#include "fprime_test_data.hpp"
#include <c25519/fprime.hpp>

#include <test_utils.hpp>
#include <cstdio>
#include <cstdlib>

using namespace ub::crypto::impl;

static void assertEquals(const uint256_t &a, const uint8_t *e, size_t sample, const char *name) {
    if (std::memcmp(a.u8, e, uint256_t::N_U8) == 0) {
        return;
    }

    fprintf(stderr, "Fp::%s test failed at sample %zd\n", name, sample);
    printNumber("Expected", e);
    printNumber("Actual  ", a.u8);
    exit(1);
}

int main() {
    for (size_t i = 0; fp_load_tests[i] != nullptr; i++) {
        const fp_load_test *t = fp_load_tests[i];

        uint256_t x, m;

        std::memcpy(m.u8, t->m, uint256_t::N_U8);
        Fp::load(x, t->i, t->il, m);

        assertEquals(x, t->r, i, "load");
    }

    for (size_t i = 0; fp_binary_tests[i] != nullptr; i++) {
        const fp_binary_test *t = fp_binary_tests[i];

        uint256_t a, b, m, r;

        std::memcpy(a.u8, t->a, uint256_t::N_U8);
        std::memcpy(b.u8, t->b, uint256_t::N_U8);
        std::memcpy(m.u8, t->m, uint256_t::N_U8);

        r = a;
        Fp::add(r, b, m);
        assertEquals(r, t->s, i, "add");

        r = b;
        Fp::add(r, a, m);
        assertEquals(r, t->s, i, "add (reversed)");

        Fp::mul(r, a, b, m);
        assertEquals(r, t->p, i, "mul");

        Fp::mul(r, b, a, m);
        assertEquals(r, t->p, i, "mul (reversed)");
    }

    return 0;
}
