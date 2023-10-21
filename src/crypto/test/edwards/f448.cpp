#include <edwards/f448_test_data.hpp>

#include <edwards/f448.hpp>
#include <test_utils.hpp>

#include <cstdio>
#include <cstdlib>

using namespace ub::crypto::impl;

static void assertEquals(const uint448_t &actual, const uint8_t *expected, size_t sample,
                         const char *name, bool normalized = false)
{
    if (std::memcmp(actual.u8, expected, uint448_t::N_U8) == 0) {
        return;
    }

    if (!normalized) {
        uint448_t plusP;
        uint32_t carry = 0;
        for (size_t i = 0; i < uint448_t::N_U8; i++) {
            uint8_t p = i == 28 ? 0xFE : 0xFF;
            uint32_t rr = (uint32_t) expected[i] + p + carry;
            plusP.u8[i] = rr;
            carry = rr >> 8;
        }

        if (std::memcmp(actual.u8, plusP.u8, uint448_t::N_U8) == 0) {
            return;
        }
    }

    fprintf(stderr, "F448::%s test failed at sample %zd\n", name, sample);
    printNumber448("Expected", expected);
    printNumber448("Actual  ", actual.u8);
    exit(1);
}

int main() {
    for (size_t i = 0; f448_load_tests[i] != nullptr; i++) {
        const f448_load_test *t = f448_load_tests[i];

        uint448_t r;
        F448::load(r, t->v);

        assertEquals(r, t->x, i, "load", true);
    }

    for (size_t i = 0; f448_binary_tests[i] != nullptr; i++) {
        const f448_binary_test *t = f448_binary_tests[i];

        uint448_t x, y, r;

        std::memcpy(x.u8, t->x, uint448_t::N_U8);
        std::memcpy(y.u8, t->y, uint448_t::N_U8);

        F448::add(r, x, y);
        assertEquals(r, t->s, i, "add");

        F448::add(r, y, x);
        assertEquals(r, t->s, i, "add (reversed)");

        // Test subtraction via negation:
        uint448_t u;

        F448::neg(u, y);
        F448::add(r, x, u);
        assertEquals(r, t->d, i, "sub");

        F448::neg(u, x);
        F448::add(r, y, u);
        assertEquals(r, t->e, i, "sub (reversed)");

        F448::mul(r, x, y);
        assertEquals(r, t->p, i, "mul");

        F448::mul(r, y, x);
        assertEquals(r, t->p, i, "mul (reversed)");
    }

    for (size_t i = 0; f448_unary_tests[i] != nullptr; i++) {
        const f448_unary_test *t = f448_unary_tests[i];

        uint448_t x, r;
        std::memcpy(x.u8, t->x, uint448_t::N_U8);

        F448::normalize(r, x);
        assertEquals(r, t->z, i, "normalize", true);

        F448::neg(r, x);
        assertEquals(r, t->n, i, "neg");

        F448::inv(r, x);
        assertEquals(r, t->i, i, "inv");

        F448::powP34(r, x);
        assertEquals(r, t->q, i, "powP34");
    }
}
