#include <device_test.hpp>

#include <ub/crypto/edwards.hpp>

TEST_IO_VARIABLE(uint8_t key[114]);
TEST_IO_VARIABLE(uint8_t signature[114]);
TEST_IO_VARIABLE(uint8_t message[1024]);
TEST_IO_VARIABLE(size_t  messageLen);

TEST_MAIN (void) {
#if defined(TEST_ED448) && TEST_ED448
    ub::crypto::ed448::sign(key, signature, message, messageLen);
#else
    ub::crypto::ed25519::sign(key, signature, message, messageLen);
#endif
}
