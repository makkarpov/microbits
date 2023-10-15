#include "hash_test.hpp"
#include <ub/crypto/sha2.hpp>

using ub::crypto::sha256;

int main() {
    return runHashTests<sha256>("sha256", sha256_buffer, sha256_samples);
}
