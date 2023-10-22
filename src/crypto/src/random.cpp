#include <ub/crypto/random.hpp>

#include <ub/crypto/utility.hpp>

#include <cstring>
#include <algorithm>

using namespace ub::crypto;

constexpr static uint32_t STATE_KEY_PTR     = 4;
constexpr static uint32_t STATE_KEY_LENGTH  = 8;
constexpr static uint32_t KEY_LENGTH        = STATE_KEY_LENGTH * sizeof(uint32_t);

constexpr static uint32_t STATE_COUNTER_PTR = STATE_KEY_PTR + STATE_KEY_LENGTH;
constexpr static uint32_t STATE_COUNTER_LENGTH = 2;


secure_random::secure_random(): m_state {}, m_stream {} {
    std::memcpy(m_state.u32, chacha20::initialConstants, sizeof(chacha20::initialConstants));
}

secure_random::~secure_random() {
    secureZero(&m_state, sizeof(m_state));
    secureZero(&m_stream, sizeof(m_stream));
}

void secure_random::pushEntropy(const uint8_t *data, size_t length) {
    while (length != 0) {
        size_t len = std::min(length, (size_t) KEY_LENGTH);
        exclusiveOr(m_state.u32 + STATE_KEY_PTR, data, len);

        advanceBlock();

        exclusiveOr(m_state.u32 + STATE_KEY_PTR, m_stream.u32 + STATE_KEY_LENGTH, KEY_LENGTH);
        secureZero(m_stream.u32, sizeof(state_t));

        data += len;
        length -= len;
    }
}

void secure_random::generate(uint8_t *buffer, size_t length) {
    while (length != 0) {
        size_t len = std::min(length, (size_t) (state_t::LENGTH - KEY_LENGTH));
        advanceBlock();

        std::memcpy(buffer, m_stream.u32 + STATE_KEY_LENGTH, len);
        secureZero(m_stream.u32, sizeof(state_t));

        buffer += len;
        length -= len;
    }
}

void secure_random::advanceBlock() {
    chacha20::processBlock(m_stream.u32, m_state.u32);

    for (size_t i = 0; i < STATE_KEY_LENGTH; i++) {
        m_state.u32[STATE_KEY_PTR + i] ^= m_stream.u32[i];
    }

    for (size_t i = 0; i < STATE_COUNTER_LENGTH; i++) {
        uint32_t v = ++m_state.u32[STATE_COUNTER_PTR];

        if (v != 0) {
            break;
        }
    }
}
