#include <device_test.hpp>

#include <ub/crypto/edwards.hpp>

#if defined(TEST_ED448) && TEST_ED448
constexpr static size_t LENGTH = ub::crypto::x448::LENGTH;
#else
constexpr static size_t LENGTH = ub::crypto::x25519::LENGTH;
#endif

TEST_IO_VARIABLE(uint8_t privateKey[LENGTH]);
TEST_IO_VARIABLE(uint8_t publicKey[LENGTH]);
TEST_IO_VARIABLE(uint8_t secret[LENGTH]);

TEST_MAIN (void) {
#if defined(TEST_ED448) && TEST_ED448
    ub::crypto::x448::compute(secret, privateKey, publicKey);
#else
    ub::crypto::x25519::compute(secret, privateKey, publicKey);
#endif
}
