#include <device_test.hpp>

#include <ub/crypto/edwards.hpp>

TEST_IO_VARIABLE(uint8_t key[57]);
TEST_IO_VARIABLE(uint8_t signature[114]);
TEST_IO_VARIABLE(uint8_t message[1024]);
TEST_IO_VARIABLE(size_t  messageLen);

TEST_MAIN (bool) {
#if defined(TEST_ED448) && TEST_ED448
    return ub::crypto::ed448::verify(key, signature, message, messageLen);
#else
    return ub::crypto::ed25519::verify(key, signature, message, messageLen);
#endif
}
