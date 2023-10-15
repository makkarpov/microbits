#include <device_test.hpp>

#include <ub/crypto/sha3.hpp>

using namespace ub::crypto;

TEST_IO_VARIABLE(uint8_t message[32768]);
TEST_IO_VARIABLE(size_t  messageLen);
TEST_IO_VARIABLE(uint8_t digest[sha3::DIGEST_256]);

TEST_MAIN (void) {
    sha3 ctx(sha3::DIGEST_256);
    ctx.update(message, messageLen);
    ctx.finish(digest);
}
