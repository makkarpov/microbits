#include "ed25519_test_data.hpp"
#include <edwards/ed25519.hpp>

#include <test_utils.hpp>

#include <cstdio>
#include <cstdlib>

using namespace ub::crypto::impl;

static void loadPoint(ed25519_pt &r, const uint8_t *x, const uint8_t *y) {
    memcpy(r.x.u8, x, uint256_t::N_U8);
    memcpy(r.y.u8, y, uint256_t::N_U8);
    r.project();
}

static void assertEquals(ed25519_pt &p, const uint8_t *rx, const uint8_t *ry, size_t sample, const char *name) {
    p.unproject();

    if (memcmp(p.x.u8, rx, uint256_t::N_U8) == 0 && memcmp(p.y.u8, ry, uint256_t::N_U8) == 0) {
        return;
    }

    fprintf(stderr, "ED25519::%s test failure at sample %zd\n", name, sample);
    fprintf(stderr, "Expected:\n");
    printNumber256("  X", rx);
    printNumber256("  Y", ry);

    fprintf(stderr, "Actual:\n");
    printNumber256("  X", p.x.u8);
    printNumber256("  Y", p.y.u8);

    exit(1);
}

int main() {
    for (size_t i = 0; ed25519_add_tests[i] != nullptr; i++) {
        const ed25519_add_test *t = ed25519_add_tests[i];

        ed25519_pt a, b, r;

        loadPoint(a, t->x1, t->y1);
        loadPoint(b, t->x2, t->y2);

        ED25519::add(r, a, b);

        assertEquals(r, t->xr, t->yr, i, "add");
    }

    for (size_t i = 0; ed25519_mul_tests[i] != nullptr; i++) {
        const ed25519_mul_test *t = ed25519_mul_tests[i];

        ed25519_pt a, r;
        uint256_t k;

        loadPoint(a, t->x1, t->y1);
        memcpy(k.u8, t->k, uint256_t::N_U8);

        ED25519::mul(r, a, k);

        assertEquals(r, t->xr, t->yr, i, "mul");
    }

    for (size_t i = 0; ed25519_load_tests[i] != nullptr; i++) {
        const ed25519_load_test *t = ed25519_load_tests[i];

        ed25519_pt r;

        bool valid = r.load(t->b);
        if (valid != t->valid) {
            fprintf(stderr, "ED25519::load test failure at sample %zd\n", i);
            fprintf(stderr, "Validity mismatch: expected=%d, actual=%d\n", t->valid, valid);
            exit(1);
        }

        if (t->valid) {
            assertEquals(r, t->xr, t->yr, i, "load");

            uint8_t storeBuf[32];
            r.store(storeBuf);

            if (memcmp(storeBuf, t->b, uint256_t::N_U8) != 0) {
                fprintf(stderr, "ED25519::store test failure at sample %zd\n", i);
                exit(1);
            }
        }
    }

    return 0;
}
