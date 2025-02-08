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

const uint8_t aes::sBox[] {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

aes::aes(): Rk {}, Nr(0) {
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
    for (uint32_t i = 0; i < Nr - 1U; i++) {
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

void aes_ctr::process(uint8_t *dst, const uint8_t *src, size_t length) {
    while (length != 0) {
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

        size_t len = std::min(length, (size_t) (aes::BLOCK - m_streamPos));
        exclusiveOr(dst, src, len, m_stream + m_streamPos);

        dst += len;
        src += len;
        length -= len;
        m_streamPos = (m_streamPos + len) & (aes::BLOCK - 1);
    }
}
