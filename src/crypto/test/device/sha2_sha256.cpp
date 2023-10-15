#include <device_test.hpp>

#include <ub/crypto/sha2.hpp>
using namespace ub::crypto;

TEST_IO_VARIABLE(uint8_t message[32768]);
TEST_IO_VARIABLE(size_t  messageLen);
TEST_IO_VARIABLE(uint8_t digest[sha256::OUTPUT]);

TEST_MAIN (void) {
    sha256 ctx;
    ctx.update(message, messageLen);
    ctx.finish(digest);
}