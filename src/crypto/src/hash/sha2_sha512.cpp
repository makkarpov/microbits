#include <ub/crypto/sha2.hpp>

#include <ub/crypto/utility.hpp>
#include "sha2_common.hpp"

#include <algorithm>

#define ROT(x, i)                   (((x) >> (i)) | ((x) << (64 - (i))))

using namespace ub::crypto;
using namespace ub::crypto::impl;

const uint64_t sha512::initialState[] = {
        0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
        0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};

const uint64_t sha512::roundConstants[] = {
        0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC, 0x3956C25BF348B538,
        0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118, 0xD807AA98A3030242, 0x12835B0145706FBE,
        0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2, 0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235,
        0xC19BF174CF692694, 0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
        0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5, 0x983E5152EE66DFAB,
        0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4, 0xC6E00BF33DA88FC2, 0xD5A79147930AA725,
        0x06CA6351E003826F, 0x142929670A0E6E70, 0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED,
        0x53380D139D95B3DF, 0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B,
        0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30, 0xD192E819D6EF5218,
        0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8, 0x19A4C116B8D2D0C8, 0x1E376C085141AB53,
        0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8, 0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373,
        0x682E6FF3D6B2B8A3, 0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
        0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B, 0xCA273ECEEA26619C,
        0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178, 0x06F067AA72176FBA, 0x0A637DC5A2C898A6,
        0x113F9804BEF90DAE, 0x1B710B35131C471B, 0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC,
        0x431D67C49C100D4C, 0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817
};

sha512::sha512(): m_block {}, m_state {} {
    std::memcpy(m_state, initialState, sizeof(initialState));
    m_totalBytes = 0;
}

void sha512::reset() {
    secureZero(m_block, sizeof(m_block));
    secureZero(m_state, sizeof(m_state));
    secureZero(&m_totalBytes, sizeof(m_totalBytes));
    std::memcpy(m_state, initialState, sizeof(initialState));
}

void sha512::update(const uint8_t *data, size_t length) {
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

void sha512::finish(uint8_t *digest) {
    uint32_t used = m_totalBytes & (BLOCK - 1);
    m_block[used++] = 0x80; // terminate message with '1' bit

    if (!writeSHA2Trailer(m_block, used, m_totalBytes, K_SHA512)) {
        processBlock();
        writeSHA2Trailer(m_block, 0, m_totalBytes, K_SHA512);
    }

    processBlock();

    for (uint32_t i = 0; i < W_STATE; i++) {
        ((uint64_t *) digest)[i] = __builtin_bswap64(m_state[i]);
    }

    reset();
}

void sha512::processBlock() {
    uint64_t w[W_ROUND];

    for (uint32_t i = 0; i < W_BLOCK; i++) {
        uint64_t wi = ((uint64_t *) m_block)[i];
        w[i] = __builtin_bswap64(wi); // assume little-endian machine
    }

    for (uint32_t i = W_BLOCK; i < W_ROUND; i++) {
        uint64_t w0 = w[i - 15];
        uint64_t s0 = ROT(w0, 1) ^ ROT(w0, 8) ^ (w0 >> 7);

        uint64_t w1 = w[i - 2];
        uint64_t s1 = ROT(w1, 19) ^ ROT(w1, 61) ^ (w1 >> 6);

        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint64_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    uint64_t e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];

    for (uint32_t i = 0; i < W_ROUND; i++) {
        uint64_t s1 = ROT(e, 14) ^ ROT(e, 18) ^ ROT(e, 41);
        uint64_t ch = (e & f) ^ (~e & g);
        uint64_t t1 = h + s1 + ch + roundConstants[i] + w[i];

        uint64_t s2 = ROT(a, 28) ^ ROT(a, 34) ^ ROT(a, 39);
        uint64_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint64_t t2 = s2 + maj;

        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}
