#include <ub/crypto/sha3.hpp>

#include <cstring>

using namespace ub::crypto;

static uint8_t sha3_rate(uint32_t digestLength) {
    if (digestLength == 0) {
        return 0;
    }

    return keccak1600::LENGTH - (digestLength << 1);
}

sha3::sha3(uint32_t digestLength): k {} {
    k.rate = sha3_rate(digestLength);
}

void sha3::reset(size_t digestLength) {
    if (digestLength != 0) {
        k.rate = sha3_rate(digestLength);
    }

    k.reset();
}

void sha3::finish(uint8_t *digest) {
    if (k.rate == 0) {
        return;
    }

    k.finish(0x06); // SHA-3 suffix '01' followed by '1' padding bit

    uint32_t digestLen = (keccak1600::LENGTH - k.rate) >> 1;
    memcpy(digest, k.st.u8, digestLen);

    reset();
}
