#include "ed25519.hpp"

using namespace ub::crypto::impl;

static constexpr uint256_t ed25519_d {
    uint256_t::from_u8,
    0xa3, 0x78, 0x59, 0x13, 0xca, 0x4d, 0xeb, 0x75, 0xab, 0xd8, 0x41, 0x41, 0x4d, 0x0a, 0x70, 0x00,
    0x98, 0xe8, 0x79, 0x77, 0x79, 0x40, 0xc7, 0x8c, 0x73, 0xfe, 0x6f, 0x2b, 0xee, 0x6c, 0x03, 0x52
};

const fp8_field_t ub::crypto::impl::C25519_ORDER {
    .len = uint256_t::N_U8,
    .mod = (const uint8_t []) {
        0xED, 0xD3, 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, 0xD6, 0x9C, 0xF7, 0xA2, 0xDE, 0xF9, 0xDE, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
    }
};

void ed25519_pt::project() {
    z = 1;
    F25519::mul(t, x, y);
}

bool ed25519_pt::load(const uint8_t *buffer) {
    // Unpack y
    if (y.u8 != buffer) {
        std::memcpy(y.u8, buffer, uint256_t::N_U8);
        y.u8[31] &= 0x7F;
    }

    uint256_t a, b, c;
    z = 1;  // Use Z as temporary location to store 1

    // Compute c = y^2
    F25519::mul(c, y, y);

    // Compute b = (1+dy^2)^-1 = (1+dc)^-1
    F25519::mul(b, c, ed25519_d);
    F25519::add(a, b, z);
    F25519::inv(b, a);

    // Compute a = y^2 - 1
    F25519::sub(a, c, z);

    // Compute c = a*b = (y^2 - 1)/(1 - dy^2)
    F25519::mul(c, a, b);

    // Compute a, b = +/- sqrt(c), if c is square
    F25519::sqrt(a, c);
    F25519::neg(b, a);

    // Select one of them, based on the compressed parity bit
    uint8_t expected_parity = buffer[31] >> 7;
    uint8_t real_parity = a.u8[0] & 1;
    x.select((bool) (expected_parity ^ real_parity), a, b);

    // Verify that x^2 = c
    F25519::mul(a, x, x);

    // Prepare projected coordinates. Z is already 1
    F25519::mul(t, x, y);

    // Return whether point was loaded successfully:
    F25519::normalize(a);
    F25519::normalize(c);
    return a == c;
}

void ed25519_pt::store(uint8_t *buffer) {
    unproject();

    memcpy(buffer, y.u8, uint256_t::N_U8);  // Copy Y into destination buffer
    buffer[31] |= (x.u8[0] & 1) << 7;       // Store parity bit of X
}

void ed25519_pt::unproject() {
    uint256_t u, v;

    F25519::inv(u, z);      // u = Z^-1

    F25519::mul(v, x, u);   // x = X/Z
    F25519::normalize(v);
    x = v;

    F25519::mul(v, y, u);   // y = Y/Z
    F25519::normalize(v);
    y = v;

    z = 1;                  // Z = 1 since X and Y are now affine
    F25519::mul(t, x, y);   // Recompute valid T
}

void ed25519_pt::loadNeutral() {
    x = 0;
    y = 1;
    z = 1;
    t = 0;
}

void ed25519_pt::destroy() {
    secureZero(this, sizeof(ed25519_pt));
}

void ed25519_pt::loadBase() {
    // y = 0x6666....6658
    std::memset(y.u8, 0x66, uint256_t::N_U8);
    y.u8[0] = 0x58;

    load(y.u8);
}

bool ed25519_pt::equals(const ed25519_pt &other) const {
    uint256_t u, v;

    F25519::mul(u, x, other.z);
    F25519::mul(v, other.x, z);
    F25519::normalize(u);
    F25519::normalize(v);
    bool r = u == v;

    F25519::mul(u, y, other.z);
    F25519::mul(v, other.y, z);
    F25519::normalize(u);
    F25519::normalize(v);
    return r & (u == v);
}

void ED25519::add(ed25519_pt &r, const ed25519_pt &p1, const ed25519_pt &p2) {
    uint256_t t[8]; // {0:a, 1:b, 2:c, 3:d, 4:e, 5:f, 6:g, 7:h}

    // A = (y1-x1)(y2-x2)
    F25519::sub(t[2], p1.y, p1.x);
    F25519::sub(t[3], p2.y, p2.x);
    F25519::mul(t[0], t[2], t[3]);

    // B = (y1+x1)(y2+x2)
    F25519::add(t[2], p1.y, p1.x);
    F25519::add(t[3], p2.y, p2.x);
    F25519::mul(t[1], t[2], t[3]);

    // C = T1 * 2 * d * T2
    F25519::mul(t[3], p1.t, p2.t);
    F25519::mul(t[2], t[3], ed25519_d);
    F25519::add(t[2], t[2], t[2]);

    // D = Z1 * 2 * Z2
    F25519::mul(t[3], p1.z, p2.z);
    F25519::add(t[3], t[3], t[3]);

    F25519::sub(t[4], t[1], t[0]); // E = B - A
    F25519::sub(t[5], t[3], t[2]); // F = D - C
    F25519::add(t[6], t[3], t[2]); // G = D + C
    F25519::add(t[7], t[1], t[0]); // H = B + A

    F25519::mul(r.x, t[4], t[5]); // X3 = E F
    F25519::mul(r.y, t[6], t[7]); // Y3 = G H
    F25519::mul(r.t, t[4], t[7]); // T3 = E H
    F25519::mul(r.z, t[5], t[6]); // Z3 = F G

    secureZero(t, sizeof(t));
}

void ED25519::mul(ed25519_pt &r, const ed25519_pt &x, const uint256_t &k) {
    ed25519_pt s;
    r.loadNeutral();

    for (int32_t i = 255; i >= 0; i--) {
        ED25519::add(r, r, r);
        ED25519::add(s, r, x);

        bool bit = (k.u8[i >> 3] >> (i & 7)) & 1;
        r.x.select(bit, r.x, s.x);
        r.y.select(bit, r.y, s.y);
        r.z.select(bit, r.z, s.z);
        r.t.select(bit, r.t, s.t);
    }
}
