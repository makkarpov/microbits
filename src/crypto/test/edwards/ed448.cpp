#include <edwards/ed448_test_data.hpp>

#include <edwards/ed448.hpp>

#include <test_utils.hpp>

#include <cstdio>
#include <cstdlib>

using namespace ub::crypto::impl;

static void loadPoint(ed448_pt &r, const uint8_t *x, const uint8_t *y) {
    std::memcpy(r.x.u8, x, uint448_t::N_U8);
    std::memcpy(r.y.u8, y, uint448_t::N_U8);
    r.z = 1;
}

static void assertEquals(ed448_pt &r, const uint8_t *x, const uint8_t *y, size_t i, const char *name) {
    r.unproject();

    if (std::memcmp(r.x.u8, x, uint448_t::N_U8) == 0 && std::memcmp(r.y.u8, y, uint448_t::N_U8) == 0) {
        return;
    }

    fprintf(stderr, "ED448::%s test failed at sample %zd\n", name, i);

    printNumber448("Expected X", x);
    printNumber448("Actual   X", r.x.u8);
    printNumber448("Expected Y", y);
    printNumber448("Actual   Y", r.y.u8);

    exit(1);
}

int main() {
    for (size_t i = 0; ed448_add_tests[i] != nullptr; i++) {
        const ed448_add_test *t = ed448_add_tests[i];
        ed448_pt r, a, b;

        loadPoint(a, t->x1, t->y1);
        loadPoint(b, t->x2, t->y2);

        ED448::add(r, a, b);

        assertEquals(r, t->xr, t->yr, i, "add");
    }

    for (size_t i = 0; ed448_mul_tests[i] != nullptr; i++) {
        const ed448_mul_test *t = ed448_mul_tests[i];
        ed448_pt r, a;
        uint448_t k;

        loadPoint(a, t->xi, t->yi);
        std::memcpy(k.u8, t->k, uint448_t::N_U8);

        ED448::mul(r, a, k);

        assertEquals(r, t->xr, t->yr, i, "mul");
    }

    for (size_t i = 0; ed448_load_tests[i] != nullptr; i++) {
        const ed448_load_test *t = ed448_load_tests[i];
        constexpr static uint32_t COMPRESSED_LEN = sizeof(t->b);

        ed448_pt r;

        bool valid = r.load(t->b);
        if (valid != t->valid) {
            fprintf(stderr, "F448::load (validity) test failed at sample %zd\n", i);
            fprintf(stderr, "Expected validity: %d\n", t->valid);
            fprintf(stderr, "Actual validity:   %d\n", valid);
            exit(1);
        }

        if (valid) {
            assertEquals(r, t->x, t->y, i, "load");

            uint8_t compressed[COMPRESSED_LEN];
            r.store(compressed);

            if (std::memcmp(compressed, t->b, COMPRESSED_LEN) != 0) {
                fprintf(stderr, "F448::store test failed at sample %zd\n", i);
                exit(1);
            }
        }
    }

    return 0;
}