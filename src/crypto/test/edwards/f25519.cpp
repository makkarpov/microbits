#include <edwards/f25519.hpp>
#include <edwards/f25519_test_data.hpp>

#include <test_utils.hpp>

#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace ub::crypto::impl;

static const uint8_t p[32] = {
        0xED, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0x7F
};
static void assertEquals(const uint256_t &actual, const uint8_t *expected, size_t sample,
                         const char *name, bool normalized = false)
{
    if (memcmp(actual.u8, expected, uint256_t::N_U8) == 0) {
        return;
    }

    // Correct test answers are always normalized mod P
    // F25519 functions are allowed to produce denormalized values mod 2P
    // To check this case, add P to the expected value and try to compare again:
    // Don't do this step when normalization routine itself is tested
    if (!normalized) {
        uint8_t expectedP[uint256_t::N_U8];
        uint32_t c = 0;

        for (size_t i = 0; i < uint256_t::N_U8; i++) {
            c = c + (uint32_t) expected[i] + (uint32_t) p[i];
            expectedP[i] = c;
            c >>= 8;
        }

        if (c == 0 && memcmp(actual.u8, expectedP, uint256_t::N_U8) == 0) {
            return;
        }
    }

    fprintf(stderr, "F25519::%s test failed at sample %zd\n", name, sample);
    printNumber256("Actual  ", actual.u8);
    printNumber256("Expected", expected);
    exit(1);
}

int main() {
    for (size_t i = 0; f25519_norm_tests[i] != nullptr; i++) {
        const f25519_norm_test *t = f25519_norm_tests[i];

        uint256_t x;
        memcpy(x.u8, t->x, uint256_t::N_U8);

        F25519::normalize(x);
        assertEquals(x, t->y, i, "normalize", true);
    }

    for (size_t i = 0; f25519_binary_tests[i] != nullptr; i++) {
        const f25519_binary_test *t = f25519_binary_tests[i];

        uint256_t x, y, r;
        memcpy(x.u8, t->x, uint256_t::N_U8);
        memcpy(y.u8, t->y, uint256_t::N_U8);

        F25519::add(r, x, y);
        assertEquals(r, t->s, i, "add");

        F25519::add(r, y, x);
        assertEquals(r, t->s, i, "add (reverse)");

        F25519::sub(r, x, y);
        assertEquals(r, t->d, i, "sub");

        F25519::sub(r, y, x);
        assertEquals(r, t->e, i, "sub (reverse)");

        F25519::mul(r, x, y);
        assertEquals(r, t->p, i, "mul");

        F25519::mul(r, y, x);
        assertEquals(r, t->p, i, "mul (reverse)");
    }

    for (size_t i = 0; f25519_unary_tests[i] != nullptr; i++) {
        const f25519_unary_test *t = f25519_unary_tests[i];

        uint256_t x, r;
        memcpy(x.u8, t->x, uint256_t::N_U8);

        F25519::neg(r, x);
        assertEquals(r, t->n, i, "neg");

        F25519::inv(r, x);
        assertEquals(r, t->i, i, "inv");
    }

    return 0;
}
