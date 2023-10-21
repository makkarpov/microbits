#include "fprime_test_data.hpp"
#include <edwards/fprime8.hpp>

#include <test_utils.hpp>
#include <cstdio>
#include <cstdlib>

using namespace ub::crypto::impl;

static void assertEquals(const uint8_t *a, const uint8_t *e, size_t len, size_t sample, const char *name) {
    if (std::memcmp(a, e, len) == 0) {
        return;
    }

    fprintf(stderr, "Fp::%s test failed at sample %zd\n", name, sample);
    printNumber256("Expected", e);
    printNumber256("Actual  ", a);
    exit(1);
}

int main() {
    for (size_t i = 0; fp_load_tests[i] != nullptr; i++) {
        const fp_load_test *t = fp_load_tests[i];
        fp8_field_t field { t->ml, t->m };

        uint8_t x[t->ml], m[t->ml];

        std::memcpy(m, t->m, t->ml);
        Fp8::load(x, t->i, t->il, field);

        assertEquals(x, t->r, t->ml, i, "load");
    }

    for (size_t i = 0; fp_binary_tests[i] != nullptr; i++) {
        const fp_binary_test *t = fp_binary_tests[i];
        fp8_field_t field { t->ml, t->m };

        uint8_t a[t->ml], b[t->ml], r[t->ml];

        std::memcpy(a, t->a, t->ml);
        std::memcpy(b, t->b, t->ml);

        std::memcpy(r, a, t->ml);
        Fp8::add(r, b, field);
        assertEquals(r, t->s, t->ml, i, "add");

        std::memcpy(r, b, t->ml);
        Fp8::add(r, a, field);
        assertEquals(r, t->s, t->ml, i, "add (reversed)");

        Fp8::mul(r, a, b, field);
        assertEquals(r, t->p, t->ml, i, "mul");

        Fp8::mul(r, b, a, field);
        assertEquals(r, t->p, t->ml, i, "mul (reversed)");
    }

    return 0;
}
