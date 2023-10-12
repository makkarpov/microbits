#include <device_test.hpp>

#include <ub/crypto/c25519.hpp>

TEST_IO_VARIABLE(uint8_t key[64]);
TEST_IO_VARIABLE(uint8_t signature[64]);
TEST_IO_VARIABLE(uint8_t message[1024]);
TEST_IO_VARIABLE(size_t  messageLen);

TEST_MAIN (void) {
    ub::crypto::ed25519::sign(key, signature, message, messageLen);
}
