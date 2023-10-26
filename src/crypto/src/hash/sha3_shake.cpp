#include <ub/crypto/sha3.hpp>

using namespace ub::crypto;

static uint32_t shake_rate(uint32_t variant) {
    if (variant == 0) {
        return 0;
    }

    return keccak1600::LENGTH - (16 << variant);
}

shake::shake(uint32_t variant): k {} {
    k.rate = shake_rate(variant);
    m_generating = false;
}

void shake::reset(uint32_t variant) {
    if (variant != 0) {
        k.rate = shake_rate(variant);
    }

    k.reset();
    m_generating = false;
}

void shake::update(const uint8_t *data, size_t length) {
    if (m_generating) {
        reset();
    }

    k.consume(data, length);
}

void shake::generate(uint8_t *output, size_t length) {
    if (!m_generating) {
        k.finish(0x1F);     // SHAKE suffix '1111' followed by '1' padding bit
        m_generating = true;
    }

    k.produce(output, length);
}
