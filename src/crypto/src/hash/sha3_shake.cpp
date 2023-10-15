#include <ub/crypto/sha3.hpp>

#include <ub/crypto/utility.hpp>

#include <cstring>

using namespace ub::crypto;

shake::shake(uint32_t variant): m_state {} {
    m_ptr = 0;
    m_variant = variant;
    m_generating = false;
}

void shake::reset(uint32_t variant) {
    secureZero(m_state.data(), keccak1600::LENGTH);
    m_ptr = 0;
    m_generating = false;

    if (variant != 0) {
        m_variant = variant;
    }
}

void shake::update(const uint8_t *data, size_t length) {
    if (m_variant == 0) {
        return;
    }

    if (m_generating) {
        reset();
    }

    process((uint8_t *) data, length, true);
}

void shake::generate(uint8_t *output, size_t length) {
    if (m_variant == 0) {
        return;
    }

    uint32_t rate = keccak1600::LENGTH - (16 << m_variant);

    if (!m_generating) {
        m_state[m_ptr]      ^= 0x1F;    // SHAKE suffix '1111' followed by '1' padding bit
        m_state[rate - 1]   ^= 0x80;    // Final '1' padding bit

        keccak1600::apply(m_state);
        m_ptr = 0;
        m_generating = true;
    }

    process(output, length, false);
}

void shake::process(uint8_t *buffer, size_t length, bool consume) {
    uint32_t rate = keccak1600::LENGTH - (16 << m_variant);
    while (length != 0) {
        size_t len = std::min(length, (size_t) (rate - m_ptr));

        if (consume) {
            exclusiveOr(m_state.data() + m_ptr, buffer, len);
        } else {
            std::memcpy(buffer, m_state.data() + m_ptr, len);
        }

        buffer += len;
        length -= len;
        m_ptr += len;

        if (m_ptr == rate) {
            keccak1600::apply(m_state);
            m_ptr = 0;
        }
    }
}
