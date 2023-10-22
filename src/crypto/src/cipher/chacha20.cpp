#include <ub/crypto/chacha20.hpp>

#include <cstring>
#include <algorithm>

#include <ub/crypto/utility.hpp>

using namespace ub::crypto;

static constexpr size_t CHACHA20_N_STATE = 16;
static constexpr size_t CHACHA20_N_ROUNDS = 10;

static constexpr size_t CHACHA20_ST_PTR_KEY     = 4;
static constexpr size_t CHACHA20_ST_PTR_BLK     = 12;
static constexpr size_t CHACHA20_ST_PTR_NONCE   = 13;

/** Quarter-round state indices encoded as 4 nibbles */
static const uint16_t chacha20_qr_indices[] = { 0x048C, 0x159D, 0x26AE, 0x37BF, 0x05AF, 0x16BC, 0x278D, 0x349E };

/** ChaCha20 constant 'expand 32-byte k' as little endian integers */
const uint32_t chacha20::initialConstants[] = { 0x61707865, 0x3320646E, 0x79622D32, 0x6B206574 };

static uint32_t chacha20_rotl(uint32_t x, uint32_t i) {
    return (x << i) | (x >> (32 - i));
}

static void chacha20_quarter_round(uint32_t *state, uint32_t indices) {
    uint32_t ai = (indices >> 12) & 0xF;
    uint32_t bi = (indices >>  8) & 0xF;
    uint32_t ci = (indices >>  4) & 0xF;
    uint32_t di = (indices >>  0) & 0xF;

    uint32_t a = state[ai];
    uint32_t b = state[bi];
    uint32_t c = state[ci];
    uint32_t d = state[di];

    a += b; d ^= a; d = chacha20_rotl(d, 16);
    c += d; b ^= c; b = chacha20_rotl(b, 12);
    a += b; d ^= a; d = chacha20_rotl(d, 8);
    c += d; b ^= c; b = chacha20_rotl(b, 7);

    state[ai] = a;
    state[bi] = b;
    state[ci] = c;
    state[di] = d;
}

void chacha20::processBlock(uint32_t *state, const uint32_t *src) {
    std::memcpy(state, src, CHACHA20_N_STATE * sizeof(uint32_t));

    for (size_t i = 0; i < CHACHA20_N_ROUNDS; i++) {
        for (uint16_t idx: chacha20_qr_indices) {
            chacha20_quarter_round(state, idx);
        }
    }

    for (size_t i = 0; i < CHACHA20_N_STATE; i++) {
        state[i] += src[i];
    }
}

chacha20::chacha20(): m_state {}, m_stream {} {
    std::memcpy(m_state.u32, initialConstants, sizeof(initialConstants));
    m_streamPtr = 0;
}

chacha20::~chacha20() {
    secureZero(m_state.u8, sizeof(m_state));
    secureZero(m_stream.u8, sizeof(m_stream));
}

void chacha20::init(const uint8_t *key, const uint8_t *nonce) {
    std::memcpy(m_state.u32 + CHACHA20_ST_PTR_KEY, key, KEY_LENGTH);
    std::memcpy(m_state.u32 + CHACHA20_ST_PTR_NONCE, nonce, NONCE_LENGTH);
    m_state.u32[CHACHA20_ST_PTR_BLK] = 0;

    processBlock();
}

void chacha20::init(const uint8_t *key, uint64_t nonce) {
    std::memcpy(m_state.u32 + CHACHA20_ST_PTR_KEY, key, KEY_LENGTH);

    m_state.u32[CHACHA20_ST_PTR_BLK] = 0;
    m_state.u32[CHACHA20_ST_PTR_NONCE + 0] = 0;
    m_state.u32[CHACHA20_ST_PTR_NONCE + 1] = (uint32_t) (nonce >> 32);
    m_state.u32[CHACHA20_ST_PTR_NONCE + 2] = (uint32_t) nonce;

    processBlock();
}

void chacha20::process(uint8_t *dst, const uint8_t *src, size_t length) {
    while (length != 0) {
        size_t len = std::min(length, (size_t) (state_t::LENGTH - m_streamPtr));
        exclusiveOr(dst, src, len, m_stream.u8 + m_streamPtr);

        dst += len;
        src += len;
        length -= len;

        m_streamPtr += len;
        if (m_streamPtr == state_t::LENGTH) {
            processBlock();
        }
    }
}

void chacha20::processBlock() {
    processBlock(m_stream.u32, m_state.u32);
    m_streamPtr = 0;

    m_state.u32[CHACHA20_ST_PTR_BLK]++;
}
