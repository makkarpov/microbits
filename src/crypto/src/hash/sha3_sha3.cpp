#include <ub/crypto/sha3.hpp>

#include <ub/crypto/utility.hpp>
#include <cstring>

using namespace ub::crypto;

sha3::sha3(uint32_t digestLength): m_state {} {
    m_digestLen = digestLength;
    m_ptr = 0;
}

void sha3::reset(size_t digestLength) {
    if (digestLength != 0) {
        m_digestLen = digestLength;
    }

    secureZero(m_state.data(), keccak1600::LENGTH);
    m_ptr = 0;
}

void sha3::update(const uint8_t *data, size_t length) {
    if (m_digestLen == 0) {
        return;
    }

    uint32_t rate = keccak1600::LENGTH - (m_digestLen << 1);
    while (length != 0) {
        size_t len = std::min(length, (size_t) (rate - m_ptr));
        exclusiveOr(m_state.data() + m_ptr, data, len);

        data += len;
        length -= len;
        m_ptr += len;

        if (m_ptr == rate) {
            keccak1600::apply(m_state);
            m_ptr = 0;
        }
    }
}

void sha3::finish(uint8_t *digest) {
    if (m_digestLen == 0) {
        return;
    }

    uint32_t rate = keccak1600::LENGTH - (m_digestLen << 1);

    // We have at least one free byte due to 'update' function logic
    m_state[m_ptr]      ^= 0x06;    // SHA-3 suffix '01' followed by '1' padding bit
    m_state[rate - 1]   ^= 0x80;    // Final '1' padding bit
    keccak1600::apply(m_state);

    memcpy(digest, m_state.data(), m_digestLen);
    reset();
}
