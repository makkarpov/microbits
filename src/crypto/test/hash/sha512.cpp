#include "hash_test.hpp"
#include <ub/crypto/sha2.hpp>

using ub::crypto::sha512;

int main() {
    return runHashTests<sha512>("sha512", sha512_buffer, sha512_samples);
}
