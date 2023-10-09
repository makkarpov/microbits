#include "ed25519.hpp"

using namespace ub::crypto::impl;

static constexpr uint256_t ed25519_d {
    uint256_t::from_u8,
    0xa3, 0x78, 0x59, 0x13, 0xca, 0x4d, 0xeb, 0x75, 0xab, 0xd8, 0x41, 0x41, 0x4d, 0x0a, 0x70, 0x00,
    0x98, 0xe8, 0x79, 0x77, 0x79, 0x40, 0xc7, 0x8c, 0x73, 0xfe, 0x6f, 0x2b, 0xee, 0x6c, 0x03, 0x52
};

static constexpr uint8_t base_x[32] {
    0x1A, 0xD5, 0x25, 0x8F, 0x60, 0x2D, 0x56, 0xC9, 0xB2, 0xA7, 0x25, 0x95, 0x60, 0xC7, 0x2C, 0x69,
    0x5C, 0xDC, 0xD6, 0xFD, 0x31, 0xE2, 0xA4, 0xC0, 0xFE, 0x53, 0x6E, 0xCD, 0xD3, 0x36, 0x69, 0x21
};

void ed25519_pt::project() {
    z = 1;
    F25519::mul(t, x, y);
}

bool ed25519_pt::load(const uint8_t *buffer) {
    // Unpack y
    std::memcpy(y.u8, buffer, uint256_t::N_U8);
    y.u8[31] &= 0x7F;

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
    std::memcpy(x.u8, base_x, uint256_t::N_U8);

    // y = 0x6666....6658
    std::memset(y.u8, 0x66, uint256_t::N_U8);
    y.u8[0] = 0x58;

    z = 1;
    F25519::mul(t, x, y);
}

void ED25519::add(ed25519_pt &r, const ed25519_pt &p1, const ed25519_pt &p2) {
    uint256_t a, b, c, d, e, f, g, h;

    // A = (y1-x1)(y2-x2)
    F25519::sub(c, p1.y, p1.x);
    F25519::sub(d, p2.y, p2.x);
    F25519::mul(a, c, d);

    // B = (y1+x1)(y2+x2)
    F25519::add(c, p1.y, p1.x);
    F25519::add(d, p2.y, p2.x);
    F25519::mul(b, c, d);

    // C = T1 * 2 * d * T2
    F25519::mul(d, p1.t, p2.t);
    F25519::mul(c, d, ed25519_d);
    F25519::add(c, c, c);

    // D = Z1 * 2 * Z2
    F25519::mul(d, p1.z, p2.z);
    F25519::add(d, d, d);

    F25519::sub(e, b, a); // E = B - A
    F25519::sub(f, d, c); // F = D - C
    F25519::add(g, d, c); // G = D + C
    F25519::add(h, b, a); // H = B + A

    F25519::mul(r.x, e, f); // X3 = E F
    F25519::mul(r.y, g, h); // Y3 = G H
    F25519::mul(r.t, e, h); // T3 = E H
    F25519::mul(r.z, f, g); // Z3 = F G

    a.destroy();
    b.destroy();
    c.destroy();
    d.destroy();
    e.destroy();
    f.destroy();
    g.destroy();
    h.destroy();
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
