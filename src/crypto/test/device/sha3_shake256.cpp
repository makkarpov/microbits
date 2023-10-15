#include <device_test.hpp>

#include <ub/crypto/sha3.hpp>

using namespace ub::crypto;

TEST_IO_VARIABLE(uint8_t message[32768]);
TEST_IO_VARIABLE(size_t  messageLen);
TEST_IO_VARIABLE(uint8_t digest[1024]);
TEST_IO_VARIABLE(size_t  digestLen);

TEST_MAIN (void) {
    shake ctx(shake::FN_SHAKE256);
    ctx.update(message, messageLen);
    ctx.generate(digest, digestLen);
}
