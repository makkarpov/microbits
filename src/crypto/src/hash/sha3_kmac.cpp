#include <ub/crypto/kmac.hpp>

using namespace ub::crypto;

// bytepad(encode_string(N) || encode_string(S), rate) with rate left as zero byte, N='KMAC', S=''
static const uint8_t KMAC_PREFIX[] = {
        0x01, 0x20, 0x4B, 0x4D, 0x41, 0x43,     // encode_string('KMAC')
        0x01, 0x00                              // encode_string('')
};

static uint32_t kmac_rate(uint32_t variant) {
    return keccak1600::LENGTH - (32 << variant);
}

kmac::kmac(): k {} {
    m_macLength = 0;
}

static void kmac_push_rate(keccak1600 &k) {
    uint8_t buf[2] {
        0x01,
        k.rate
    };

    k.consume(buf, sizeof(buf));
}

static uint8_t kmac_encoded_length(size_t x) {
    x = x | 1;   // at least one byte must be serialized

    uint8_t r = 0;
    while (x != 0) {
        r++;
        x >>= 8;
    }

    return r;
}

static void kmac_left_encode(keccak1600 &k, size_t x) {
    uint8_t buf[sizeof(size_t) + 1];

    size_t l = kmac_encoded_length(x);
    buf[0] = l;

    size_t i = l;
    while (i != 0) {
        buf[i] = (uint8_t) x;
        i--;
        x >>= 8;
    }

    k.consume(buf, l + 1);
}

static void kmac_right_encode(keccak1600 &k, size_t x) {
    uint8_t buf[sizeof(size_t) + 1];

    size_t l = kmac_encoded_length(x);
    buf[l] = l;

    size_t i = l;
    while (i != 0) {
        i--;
        buf[i] = (uint8_t) x;
        x >>= 8;
    }

    k.consume(buf, l + 1);
}

void kmac::init(uint32_t variant, const uint8_t *key, size_t keyLength, size_t macLength) {
    k.rate = kmac_rate(variant);
    k.reset();

    // cSHAKE step: add bytepad(encode_string(N) || encode_string(S), rate) prefix
    kmac_push_rate(k);
    k.consume(KMAC_PREFIX, sizeof(KMAC_PREFIX));

    keccak1600::apply(k.st);
    k.ptr = 0;

    // KMAC step: add bytepad(encode_string(K), rate)

    kmac_push_rate(k);
    kmac_left_encode(k, keyLength << 3);
    k.consume(key, keyLength);

    if (k.ptr != 0) {
        keccak1600::apply(k.st);
        k.ptr = 0;
    }

    m_macLength = macLength;
}

void kmac::finish(uint8_t *mac) {
    // KMAC step: add right_encode(L)
    kmac_right_encode(k, m_macLength << 3);

    k.finish(0x04); // cSHAKE suffix '00' followed by '1' padding bit
    k.produce(mac, m_macLength);
}
