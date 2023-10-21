#include "ed25519.hpp"

using namespace ub::crypto::impl;

static constexpr uint256_t ed25519_d {
    uint256_t::from_u8,
    0xa3, 0x78, 0x59, 0x13, 0xca, 0x4d, 0xeb, 0x75, 0xab, 0xd8, 0x41, 0x41, 0x4d, 0x0a, 0x70, 0x00,
    0x98, 0xe8, 0x79, 0x77, 0x79, 0x40, 0xc7, 0x8c, 0x73, 0xfe, 0x6f, 0x2b, 0xee, 0x6c, 0x03, 0x52
};

static const uint8_t ed25519_base_x[32] {
    0x1A, 0xD5, 0x25, 0x8F, 0x60, 0x2D, 0x56, 0xC9, 0xB2, 0xA7, 0x25, 0x95, 0x60, 0xC7, 0x2C, 0x69,
    0x5C, 0xDC, 0xD6, 0xFD, 0x31, 0xE2, 0xA4, 0xC0, 0xFE, 0x53, 0x6E, 0xCD, 0xD3, 0x36, 0x69, 0x21
};

static constexpr uint256_t ed25519_sqrt_k { // 2^((p-1)/4) mod p
    uint256_t::from_u8,
    0xB0, 0xA0, 0x0E, 0x4A, 0x27, 0x1B, 0xEE, 0xC4, 0x78, 0xE4, 0x2F, 0xAD, 0x06, 0x18, 0x43, 0x2F,
    0xA7, 0xD7, 0xFB, 0x3D, 0x99, 0x00, 0x4D, 0x2B, 0x0B, 0xDF, 0xC1, 0x4F, 0x80, 0x24, 0x83, 0x2B
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

    uint256_t tt[5];
    z = 1;

    // Compute candidate root:
    F25519::mul(tt[0], y, y);               // tt[0]: y^2
    F25519::sub(tt[1], tt[0], z);           // tt[1]: u = y^2 - 1
    F25519::normalize(tt[1]);

    F25519::mul(tt[2], tt[0], ed25519_d);   // tt[2]: d y^2
    F25519::add(tt[0], tt[2], z);           // tt[0]: v = d y^2 + 1

    F25519::mul(tt[2], tt[0], tt[0]);       // tt[2]: v^2
    F25519::mul(tt[3], tt[2], tt[2]);       // tt[3]: v^4
    F25519::mul(tt[4], tt[3], tt[2]);       // tt[4]: v^6
    F25519::mul(tt[2], tt[4], tt[0]);       // tt[2]: v^7
    F25519::mul(tt[3], tt[2], tt[1]);       // tt[3]: u v^7
    F25519::pow58(tt[2], tt[3]);            // tt[2]: (u v^7)^((p-5)/8)
    F25519::mul(tt[3], tt[2], tt[1]);       // tt[3]: u P58(u v^7)
    F25519::mul(tt[2], tt[0], tt[0]);       // tt[2]: v^2
    F25519::mul(tt[4], tt[2], tt[0]);       // tt[4]: v^3
    F25519::mul(tt[2], tt[4], tt[3]);       // tt[2]: x (candidate)

    // Compute v x^2 to check whether it's a root
    F25519::mul(tt[3], tt[2], tt[2]);       // tt[3]: x^2
    F25519::mul(tt[4], tt[3], tt[0]);       // tt[4]: v x^2
    F25519::normalize(tt[4]);

    F25519::mul(tt[3], tt[2], ed25519_sqrt_k);  // tt[3]: x * 2^((p-1)/4)
    // used tt slots at this point:
    // [1]: u, [2]: x, [3]: x2, [4]: v x^2

    F25519::neg(tt[0], tt[1]);
    F25519::normalize(tt[0]);               // tt[0]: -u

    bool case1 = tt[4] == tt[1];
    bool case2 = tt[4] == tt[0];
    if (!(case1 | case2)) {
        return false;
    }

    tt[0].select(case1, tt[3], tt[2]);      // tt[0]: x (final sqrt)
    F25519::neg(tt[1], tt[0]);

    uint8_t expectedParity = buffer[31] >> 7;
    uint8_t realParity = tt[0].u8[0] & 1;
    x.select((bool) (expectedParity ^ realParity), tt[0], tt[1]);

    F25519::mul(t, x, y);
    return true;
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

    std::memcpy(x.u8, ed25519_base_x, uint256_t::N_U8);

    z = 1;
    F25519::mul(t, x, y);
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

static void ed25519_add_impl(ed25519_pt &r, const ed25519_pt &p1, const ed25519_pt &p2, uint256_t *t) {
    F25519::sub(t[0], p1.y, p1.x);
    F25519::sub(t[1], p2.y, p2.x);
    F25519::mul(t[2], t[0], t[1]);  // t[2]: A=(Y1-X1)*(Y2-X2)

    F25519::add(t[0], p1.y, p1.x);
    F25519::add(t[1], p2.y, p2.x);
    F25519::mul(t[3], t[0], t[1]);  // t[3]: B=(Y1+X1)*(Y2-X2)

    F25519::sub(t[0], t[3], t[2]);  // t[0]: E=B-A
    F25519::add(t[1], t[3], t[2]);  // t[1]: H=B+A

    F25519::mul(t[2], p1.t, p2.t);
    F25519::mul(t[3], t[2], ed25519_d);
    F25519::add(t[2], t[3], t[3]);  // t[2]: C=2*d*T1*T2

    F25519::mul(t[3], p1.z, p2.z);
    F25519::add(t[3], t[3], t[3]);  // t[3]: D=2*Z1*Z2

    F25519::sub(t[4], t[3], t[2]);  // t[4]: F=D-C
    F25519::add(t[5], t[3], t[2]);  // t[5]: G=D+C

    F25519::mul(r.x, t[0], t[4]);   // X3=E*F
    F25519::mul(r.y, t[5], t[1]);   // Y3=G*H
    F25519::mul(r.z, t[5], t[4]);   // Z3=G*F
    F25519::mul(r.t, t[0], t[1]);   // T3=E*H
}

static void ed25519_double(ed25519_pt &r, const ed25519_pt &a, uint256_t *t) {
    F25519::mul(t[0], a.x, a.x);    // t[0]: A=X^2
    F25519::mul(t[1], a.y, a.y);    // t[1]: B=Y^2
    F25519::add(t[2], t[0], t[1]);  // t[2]: H=A+B
    F25519::sub(t[3], t[0], t[1]);  // t[3]: G=A-B

    F25519::mul(t[1], a.z, a.z);
    F25519::add(t[0], t[1], t[1]);  // t[0]: C=2*Z^2
    F25519::add(t[0], t[0], t[3]);  // t[0]: F=C+G

    F25519::add(t[1], a.x, a.y);
    F25519::mul(t[4], t[1], t[1]);
    F25519::sub(t[1], t[2], t[4]);  // t[1]: E=H-(X+Y)^2

    F25519::mul(r.x, t[1], t[0]);   // X3=E*F
    F25519::mul(r.y, t[3], t[2]);   // Y3=G*H
    F25519::mul(r.z, t[3], t[0]);   // Z3=G*F
    F25519::mul(r.t, t[1], t[2]);   // T3=E*H
}

void ED25519::add(ed25519_pt &r, const ed25519_pt &p1, const ed25519_pt &p2) {
    uint256_t t[6];
    ed25519_add_impl(r, p1, p2, t);
    ub::crypto::secureZero(t, sizeof(t));
}

void ED25519::mul(ed25519_pt &r, const ed25519_pt &x, const uint256_t &k) {
    uint256_t t[6];
    ed25519_pt s;

    r.loadNeutral();

    for (int32_t i = 255; i >= 0; i--) {
        ed25519_double(r, r, t);
        ed25519_add_impl(s, r, x, t);

        bool bit = (k.u8[i >> 3] >> (i & 7)) & 1;
        r.x.select(bit, r.x, s.x);
        r.y.select(bit, r.y, s.y);
        r.z.select(bit, r.z, s.z);
        r.t.select(bit, r.t, s.t);
    }

    ub::crypto::secureZero(t, sizeof(t));
}
