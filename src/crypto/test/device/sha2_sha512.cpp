#include <device_test.hpp>

#include <ub/crypto/sha2.hpp>
using namespace ub::crypto;

TEST_IO_VARIABLE(uint8_t message[32768]);
TEST_IO_VARIABLE(size_t  messageLen);
TEST_IO_VARIABLE(uint8_t digest[sha512::OUTPUT]);

static_assert(sizeof(sha512) < 1000);

TEST_MAIN (void) {
    sha512 ctx;
    ctx.update(message, messageLen);
    ctx.finish(digest);
}