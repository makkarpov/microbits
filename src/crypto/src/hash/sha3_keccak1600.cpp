#include <ub/crypto/sha3.hpp>

#include <cstring>

using namespace ub::crypto;

enum { KECCAK1600_ROUNDS = 24 };

static const uint8_t keccak1600_rotations[] = {
        1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
        27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

static const uint8_t keccak1600_permutation[] = {
        10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
        15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
};

// Compressed round constants: 15, 31 and 63 bits are packed into high byte, low byte is applied directly
static const uint16_t keccak1600_rcon[] = {
        0x0001, 0x0182, 0x058A, 0x0700, 0x018B, 0x0201, 0x0781, 0x0509, 0x008A, 0x0088, 0x0309, 0x020A,
        0x038B, 0x048B, 0x0589, 0x0503, 0x0502, 0x0480, 0x010A, 0x060A, 0x0781, 0x0580, 0x0201, 0x0708
};

static uint64_t rot64(uint64_t x, uint32_t i) {
    return (x << i) | (x >> (64 - i));
}

static void keccak1600_theta(uint32_t *st) {
    static const uint8_t selectors[5] = { 0x82, 0x04, 0x26, 0x48, 0x60 };

    uint32_t bc[10];
    for (size_t i = 0; i < 10; i++) {
        uint32_t t = 0;

        for (size_t j = 0; j < 50; j += 10) {
            t ^= st[i + j];
        }

        bc[i] = t;
    }

    for (size_t i = 0; i < 10; i++) {
        uint32_t selector = selectors[i >> 1];
        uint32_t s_u = (selector >> 4) ^ (i & 1);
        uint32_t s_v = (selector & 0xF) ^ (i & 1);

        uint32_t t = bc[s_u] ^ (bc[s_v] << 1) ^ (bc[s_v ^ 1] >> 31);
        for (size_t j = 0; j < 50; j += 10) {
            st[i + j] ^= t;
        }
    }
}

static void keccak1600_rho_pi(uint32_t *st) {
    uint64_t t = st[2] | ((uint64_t) st[3] << 32);

    for (size_t i = 0; i < 24; i++) {
        uint32_t j = keccak1600_permutation[i] << 1;
        uint64_t x = st[j] | ((uint64_t) st[j + 1] << 32);
        uint64_t y = rot64(t, keccak1600_rotations[i]);

        st[j] = y;
        st[j + 1] = y >> 32;

        t = x;
    }
}

static void keccak1600_chi(uint32_t *st) {
    uint32_t bc[14];

    for (size_t j = 0; j < 50; j += 10) {
        for (size_t i = 0; i < 10; i++) {
            bc[i] = st[j + i];
        }

        // Avoid expensive '% 10' at cost of bit of RAM
        for (size_t i = 0; i < 4; i++) {
            bc[10 + i] = bc[i];
        }

        for (size_t i = 0; i < 10; i++) {
            st[j + i] ^= ~bc[i + 2] & bc[i + 4];
        }
    }
}

void keccak1600_iota(uint32_t *st, uint32_t rc) {
    uint32_t iota0 = st[0];

    iota0 ^= rc & 0xFF;
    iota0 ^= (rc & 0x100) << 7;
    iota0 ^= (rc & 0x200) << 22;
    st[0] = iota0;

    st[1] ^= (rc & 0x400) << 21;
}

void keccak1600::apply(state_t &state) {
    for (size_t i = 0; i < KECCAK1600_ROUNDS; i++) {
        keccak1600_theta(state.u32);
        keccak1600_rho_pi(state.u32);
        keccak1600_chi(state.u32);
        keccak1600_iota(state.u32, keccak1600_rcon[i]);
    }
}

void keccak1600::consume(const uint8_t *buf, size_t length) {
    while (length != 0) {
        size_t ll = std::min(length, (size_t) (rate - ptr));
        exclusiveOr(st.u8 + ptr, buf, ll);

        ptr += ll;
        buf += ll;
        length -= ll;

        if (ptr == rate) {
            apply(st);
            ptr = 0;
        }
    }
}

void keccak1600::finish(uint8_t trailer) {
    st.u8[ptr]      ^= trailer;     // function-specific trailer field
    st.u8[rate - 1] ^= 0x80;        // final '1' padding bit
    apply(st);

    ptr = 0;
}

void keccak1600::produce(uint8_t *buf, size_t length) {
    while (length != 0) {
        size_t ll = std::min(length, (size_t) (rate - ptr));
        std::memcpy(buf, st.u8 + ptr, ll);

        ptr += ll;
        buf += ll;
        length -= ll;

        if (ptr == rate) {
            apply(st);
            ptr = 0;
        }
    }
}
