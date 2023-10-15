#include <ub/crypto/sha2.hpp>

#include <ub/crypto/utility.hpp>
#include "sha2_common.hpp"

#include <cstring>
#include <algorithm>

#define ROT(x, i)                   (((x) >> (i)) | ((x) << (32 - (i))))

using namespace ub::crypto;
using namespace ub::crypto::impl;

const uint32_t sha256::initialState[] = {
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

const uint32_t sha256::roundConstants[] = {
        0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
        0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
        0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
        0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
        0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
        0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
        0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
        0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

sha256::sha256(): m_block {}, m_state {} {
    std::memcpy(m_state, initialState, sizeof(initialState));
    m_totalBytes = 0;
}

void sha256::reset() {
    secureZero(m_block, sizeof(m_block));
    secureZero(m_state, sizeof(m_state));
    secureZero(&m_totalBytes, sizeof(m_totalBytes));
    std::memcpy(m_state, initialState, sizeof(initialState));
}

void sha256::update(const uint8_t *data, size_t length) {
    while (length != 0) {
        uint32_t used = m_totalBytes & (BLOCK - 1);
        uint32_t len = std::min(length, (size_t) (BLOCK - used));

        std::memcpy(m_block + used, data, len);

        data += len;
        length -= len;
        m_totalBytes += len;

        if ((m_totalBytes & (BLOCK - 1)) == 0) {
            processBlock();
        }
    }
}

void sha256::finish(uint8_t *digest) {
    // block is guaranteed to have at least 1 free byte due to update() logic
    uint32_t used = m_totalBytes & (BLOCK - 1);
    m_block[used++] = 0x80; // terminate message with '1' bit

    if (!writeSHA2Trailer(m_block, used, m_totalBytes, K_SHA256)) {
        processBlock();
        writeSHA2Trailer(m_block, 0, m_totalBytes, K_SHA256);
    }

    processBlock();

    for (uint32_t i = 0; i < W_STATE; i++) {
        ((uint32_t *) digest)[i] = __builtin_bswap32(m_state[i]);
    }

    reset();
}

void sha256::processBlock() {
    uint32_t w[W_ROUND];

    for (uint32_t i = 0; i < W_BLOCK; i++) {
        uint32_t wi = ((uint32_t *) m_block)[i];
        w[i] = __builtin_bswap32(wi); // assume little-endian system
    }

    for (uint32_t i = W_BLOCK; i < W_ROUND; i++) {
        uint32_t w0 = w[i - 15];
        uint32_t s0 = ROT(w0, 7) ^ ROT(w0, 18) ^ (w0 >> 3);

        uint32_t w1 = w[i - 2];
        uint32_t s1 = ROT(w1, 17) ^ ROT(w1, 19) ^ (w1 >> 10);

        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    uint32_t e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];

    for (uint32_t i = 0; i < W_ROUND; i++) {
        uint32_t s1 = ROT(e, 6) ^ ROT(e, 11) ^ ROT(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = h + s1 + ch + roundConstants[i] + w[i];

        uint32_t s2 = ROT(a, 2) ^ ROT(a, 13) ^ ROT(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = s2 + maj;

        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}
