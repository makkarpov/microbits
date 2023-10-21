#include "ed448.hpp"

#include "f448.hpp"

using namespace ub::crypto::impl;

constexpr static int32_t ED448_D = -39081;
constexpr static size_t PARITY_POS = uint448_t::N_U8;

static const uint8_t ED448_BASE_POINT[] = {
        0x14, 0xFA, 0x30, 0xF2, 0x5B, 0x79, 0x08, 0x98, 0xAD, 0xC8, 0xD7, 0x4E, 0x2C, 0x13, 0xBD, 0xFD, 0xC4, 0x39,
        0x7C, 0xE6, 0x1C, 0xFF, 0xD3, 0x3A, 0xD7, 0xC2, 0xA0, 0x05, 0x1E, 0x9C, 0x78, 0x87, 0x40, 0x98, 0xA3, 0x6C,
        0x73, 0x73, 0xEA, 0x4B, 0x62, 0xC7, 0xC9, 0x56, 0x37, 0x20, 0x76, 0x88, 0x24, 0xBC, 0xB6, 0x6E, 0x71, 0x46,
        0x3F, 0x69, 0x00
};

const fp8_field_t ub::crypto::impl::C448_ORDER {
    .len = 56,
    .mod = (const uint8_t[]) {
        0xF3, 0x44, 0x58, 0xAB, 0x92, 0xC2, 0x78, 0x23, 0x55, 0x8F, 0xC5, 0x8D, 0x72, 0xC2, 0x6C, 0x21, 0x90, 0x36,
        0xD6, 0xAE, 0x49, 0xDB, 0x4E, 0xC4, 0xE9, 0x23, 0xCA, 0x7C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x3F
    }
};

bool ed448_pt::load(const uint8_t *buffer) {
    if (buffer != y.u8) {
        std::memcpy(y.u8, buffer, uint448_t::N_U8);

        // Decoding fails if Y was greater than P
        if (F448::normalize(y, y)) {
            return false;
        }
    }

    if ((buffer[PARITY_POS] & 0x7F) != 0) {
        return false;
    }

    uint448_t u, v, t[3];

    // u = y^2 - 1; v = d y^2 - 1
    F448::mul(u, y, y);

    F448::load(t[0], ED448_D);
    F448::mul(v, u, t[0]);

    F448::load(t[0], -1);
    F448::add(u, u, t[0]);
    F448::add(v, v, t[0]);

    // x = u^3 v (u^5 v^3)^((p-3)/4)
    F448::mul(t[0], u, u);          // t[0] = u^2
    F448::mul(t[1], t[0], t[0]);    // t[1] = u^4
    F448::mul(t[0], t[1], u);       // t[0] = u^5
    F448::mul(t[1], v, v);          // t[1] = v^2
    F448::mul(t[2], t[1], v);       // t[2] = v^3
    F448::mul(t[1], t[0], t[2]);    // t[1] = u^5 v^3

    F448::powP34(t[0], t[1]);       // t[0] = P34(u^5 v^3)
    F448::mul(t[1], t[0], v);       // t[1] = v P32(u^5 v^3)
    F448::mul(t[0], u, u);          // t[0] = u^2
    F448::mul(t[2], t[0], u);       // t[2] = u^3
    F448::mul(x, t[1], t[2]);       // x = u^3 v P32(u^5 v^3)

    // Select correct X
    uint32_t parityC = (buffer[PARITY_POS] >> 7) & 1;
    uint32_t parityR = x.u32[0] & 1;
    F448::neg(t[0], x);             // t[0] = -x
    x.select(parityC ^ parityR, x, t[0]);

    // Check that X is a square
    F448::mul(t[0], x, x);          // t[0] = x^2
    F448::mul(t[1], t[0], v);       // t[1] = v x^2
    F448::normalize(t[1], t[1]);
    F448::normalize(u, u);
    bool valid = t[1] == u;

    z = 1;
    return valid;
}

void ed448_pt::store(uint8_t *buffer) {
    unproject();

    std::memcpy(buffer, y.u8, uint448_t::N_U8);
    buffer[PARITY_POS] = (x.u8[0] & 1) << 7;
}

void ed448_pt::unproject() {
    uint448_t zi, t;

    F448::inv(zi, z);

    F448::mul(t, x, zi);
    x = t;

    F448::mul(t, y, zi);
    y = t;

    z = 1;
}

void ed448_pt::loadNeutral() {
    x = 0;
    y = 1;
    z = 1;
}

void ed448_pt::loadBase() {
    load(ED448_BASE_POINT);
}

bool ed448_pt::equals(const ed448_pt &other) const {
    uint448_t u, v;

    F448::mul(u, x, other.z);
    F448::mul(v, other.x, z);
    F448::normalize(u, u);
    F448::normalize(v, v);
    bool r = u == v;

    F448::mul(u, y, other.z);
    F448::mul(v, other.y, z);
    F448::normalize(u, u);
    F448::normalize(v, v);

    return r & (u == v);
}

void ED448::add(ed448_pt &r, const ed448_pt &a, const ed448_pt &b) {
    uint448_t t[8];

    F448::mul(t[0], a.z, b.z);      // A = Z1*Z2
    F448::mul(t[1], t[0], t[0]);    // B = A^2
    F448::mul(t[2], a.x, b.x);      // C = X1*X2
    F448::mul(t[3], a.y, b.y);      // D = Y1*Y2

    // E = d*C*D
    F448::mul(t[5], t[2], t[3]);
    F448::load(t[6], ED448_D);
    F448::mul(t[4], t[5], t[6]);

    F448::add(t[6], t[1], t[4]);    // G = B+E

    F448::neg(t[4], t[4]);
    F448::add(t[5], t[1], t[4]);    // F = B-E
    // B and E are now dead and could be reused

    F448::add(t[1], a.x, a.y);
    F448::add(t[4], b.x, b.y);
    F448::mul(t[7], t[1], t[4]);    // H = (X1+Y1)*(X2+Y2)

    F448::neg(t[2], t[2]);          // Prepare -C
    F448::mul(r.z, t[5], t[6]);     // Z3 = F*G

    // Y3 = A*G*(D-C)
    F448::add(t[1], t[3], t[2]);
    F448::mul(t[4], t[1], t[6]);
    F448::mul(r.y, t[0], t[4]);

    // X3 = A*F*(H-C-D)
    F448::neg(t[3], t[3]);
    F448::add(t[7], t[7], t[2]);
    F448::add(t[7], t[7], t[3]);
    F448::mul(t[1], t[5], t[7]);
    F448::mul(r.x, t[1], t[0]);

    secureZero(t, sizeof(t));
}

static void ed448_double(ed448_pt &r, const ed448_pt &a) {
    uint448_t t[6];

    // B = (X1+Y1)^2
    F448::add(t[0], a.x, a.y);
    F448::mul(t[1], t[0], t[0]);

    F448::mul(t[2], a.x, a.x);      // C = X1^2
    F448::mul(t[3], a.y, a.y);      // D = Y1^2

    F448::add(t[4], t[2], t[3]);    // E = C+D
    F448::mul(t[5], a.z, a.z);      // H = Z1^2

    F448::add(t[5], t[5], t[5]);
    F448::neg(t[5], t[5]);
    F448::add(t[5], t[4], t[5]);    // J = E-2*H

    F448::mul(r.z, t[4], t[5]);     // Z3 = E*J

    // Y3 = E*(C-D)
    F448::neg(t[3], t[3]);
    F448::add(t[2], t[2], t[3]);
    F448::mul(r.y, t[4], t[2]);

    // X3 = J*(B-E)
    F448::neg(t[4], t[4]);
    F448::add(t[1], t[1], t[4]);
    F448::mul(r.x, t[5], t[1]);

    ub::crypto::secureZero(t, sizeof(t));
}

void ED448::mul(ed448_pt &r, const ed448_pt &x, const uint448_t &k) {
    ed448_pt s;
    r.loadNeutral();

    for (int32_t i = 447; i >= 0; i--) {
        ed448_double(r, r);
        add(s, r, x);

        bool bit = (k.u8[i >> 3] >> (i & 7)) & 1;
        r.x.select(bit, r.x, s.x);
        r.y.select(bit, r.y, s.y);
        r.z.select(bit, r.z, s.z);
    }
}
