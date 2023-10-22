#include <device_test.hpp>

#include <ub/crypto/aes.hpp>
#include <ub/crypto/chacha20.hpp>

using namespace ub::crypto;

TEST_IO_VARIABLE(uint8_t key[32]);
TEST_IO_VARIABLE(size_t  keyLength);
TEST_IO_VARIABLE(uint8_t nonce[16]);
TEST_IO_VARIABLE(uint8_t message[1024]);
TEST_IO_VARIABLE(size_t  messageLength);

TEST_MAIN (void) {
#if defined(TEST_AES) && TEST_AES
    aes_ctr ctx;
    ctx.init(key, nonce, keyLength);
    ctx.process(message, message, messageLength);
#endif

#if defined(TEST_CHACHA20) && TEST_CHACHA20
    chacha20 ctx;
    ctx.init(key, nonce);
    ctx.process(message, message, messageLength);
#endif
}
