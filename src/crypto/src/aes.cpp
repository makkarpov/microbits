#include <ub/crypto/aes.hpp>

#include <ub/crypto/utility.hpp>

#include <cstring>
#include <algorithm>

using namespace ub::crypto;

// Constant-time version of shift-and-xor GF multiplication algorithm
static uint8_t gf_double(uint8_t x) {
    int32_t h = -((x >> 7) & 1);
    return (uint8_t) (x << 1) ^ (h & 0x1B);
}

uint8_t aes::sBox[] {};

aes::aes(): Rk {}, Nr(0) {
    // TODO: Hacky but works for now
    if (sBox[0] == 0) {
        generateSBox();
    }
}

void aes::reset() {
    if (Nr == 0) {
        return;
    }

    secureZero(Rk, sizeof(Rk));
    secureZero(&Nr, sizeof(Nr));
}

bool aes::init(const uint8_t *key, size_t length) {
    reset();

    if (length != 16 && length != 24 && length != 32) {
        return false;
    }

    size_t Nk = length >> 2;
    Nr = 6 + Nk;

    uint8_t rc = 0x01;
    size_t Nrk = (Nr + 1) * (BLOCK / WORD);
    memcpy(Rk, key, length);

    for (uint32_t i = Nk; i < Nrk; i++) {
        uint8_t Wm1[WORD];
        memcpy(Wm1, Rk + (i - 1) * WORD, WORD);

        uint32_t iN = i % Nk;
        if (iN == 0) {
            for (uint32_t c = 0; c < WORD - 1; c++) {
                uint8_t t = Wm1[c];
                Wm1[c] = Wm1[c + 1];
                Wm1[c + 1] = t;
            }
        }

        if (iN == 0 || (Nk > 6 && iN == 4)) {
            // SubWord:
            for (uint8_t &c: Wm1) {
                c = sBox[c];
            }
        }

        if (iN == 0) {
            Wm1[0] ^= rc;
            rc = gf_double(rc);
        }

        for (uint32_t c = 0; c < WORD; c++) {
            Rk[i * WORD + c] = Rk[(i - Nk) * WORD + c] ^ Wm1[c];
        }
    }

    return true;
}

void aes::encrypt(uint8_t *block) {
    for (uint32_t i = 0; i < Nr - 1; i++) {
        addRoundKey(i, block);
        subShift(block);
        mixColumns(block);
    }

    addRoundKey(Nr - 1, block);
    subShift(block);
    addRoundKey(Nr, block);
}

void aes::subShift(uint8_t *state) {
    static const uint8_t shiftTable[16] = { 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11 };

    uint8_t s[BLOCK];
    memcpy(s, state, BLOCK);

    for (uint32_t i = 0; i < BLOCK; i++) {
        state[i] = sBox[s[shiftTable[i]]];
    }
}

void aes::mixColumns(uint8_t *state) {
    uint8_t a[4], b[4];
    for (uint32_t i = 0; i < BLOCK; i += WORD) {
        for (uint32_t c = 0; c < WORD; c++) {
            a[c] = state[i + c];
            b[c] = gf_double(a[c]);
        }

        for (uint32_t c = 0; c < WORD; c++) {
            uint8_t x = (c + 1) & (WORD - 1);
            uint8_t r = b[c] ^ a[x] ^ b[x];

            x = (x + 1) & (WORD - 1);
            r ^= a[x];

            x = (x + 1) & (WORD - 1);
            r ^= a[x];

            state[i + c] = r;
        }
    }
}

void aes::addRoundKey(uint32_t k, uint8_t *state) {
    const uint8_t *rk = Rk + k * BLOCK;
    for (uint32_t i = 0; i < BLOCK; i++) {
        state[i] ^= rk[i];
    }
}

void aes::generateSBox() {
    uint8_t p = 1, q = 1;

    /* loop invariant: p * q == 1 in the Galois field */
    do {
        /* multiply p by 3 */
        p = p ^ gf_double(p);

        /* divide q by 3 (equals multiplication by 0xf6) */
        q ^= q << 1;
        q ^= q << 2;
        q ^= q << 4;
        q ^= q & 0x80 ? 0x09 : 0;

        uint8_t qr = 0, qq = q;
        for (uint32_t i = 0; i < 5; i++) {
            qr ^= qq;
            qq = (qq << 1) | (qq >> 7);
        }

        sBox[p] = qr ^ 0x63;
    } while (p != 1);

    /* 0 is a special case since it has no inverse */
    sBox[0] = 0x63;
}

aes_ctr::aes_ctr(): m_counter {}, m_stream {} {
    m_streamPos = 0;
}

void aes_ctr::reset() {
    m_aes.reset();
    secureZero(m_counter, sizeof(m_counter));
    secureZero(m_stream, sizeof(m_stream));
    secureZero(&m_streamPos, sizeof(m_streamPos));
}

bool aes_ctr::init(const uint8_t *key, const uint8_t *nonce, size_t keyLength) {
    if (!m_aes.init(key, keyLength)) {
        return false;
    }

    memcpy(m_counter, nonce, aes::BLOCK);
    m_streamPos = 0;
    return true;
}

void aes_ctr::process(uint8_t *buffer, size_t length) {
    uint8_t *end = buffer + length;

    while (buffer != end) {
        if (m_streamPos == 0) {
            memcpy(m_stream, m_counter, aes::BLOCK);
            m_aes.encrypt(m_stream);

            for (int32_t i = aes::BLOCK - 1; i >= 0; i--) {
                m_counter[i]++;

                if (m_counter[i] != 0) {
                    break;
                }
            }
        }

        *buffer ^= m_stream[m_streamPos];
        m_streamPos = (m_streamPos + 1) & (aes::BLOCK - 1);
        *buffer++;
    }
}
