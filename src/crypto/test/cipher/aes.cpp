#include "block_test_data.hpp"

#include <ub/crypto/aes.hpp>

#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace ub::crypto;

int main() {
    aes aes;

    for (size_t i = 0; aes_block_tests[i] != nullptr; i++) {
        const cipher_block_test *t = aes_block_tests[i];

        aes.init(t->k, t->kl);

        uint8_t block[aes::BLOCK];

        std::memcpy(block, t->i, aes::BLOCK);
        aes.encrypt(block);

        if (std::memcmp(block, t->o, aes::BLOCK) != 0) {
            fprintf(stderr, "aes::encrypt test failed at sample %zd\n", i);
            exit(1);
        }
    }

    return 0;
}