#ifndef UB_CRYPTO_RANDOM_H
#define UB_CRYPTO_RANDOM_H

#include <cstdint>
#include <cstddef>

#include <ub/crypto/chacha20.hpp>

namespace ub::crypto {
    /**
     * Cryptographically secure pseudo random generator implemented on top of ChaCha20 block primitive.
     *
     * Instances of this class must be seeded externally before use.
     *
     * Output of this class is not a public API and could change between versions. Do not use it to derive deterministic
     * random data from long-term seeds.
     */
    class secure_random {
    public:
        /** Initialize CPRRNG instance to empty state */
        explicit secure_random();

        /** Destroy CSPRNG instance, erasing sensitive data */
        ~secure_random();

        /** Add more entropy to the RNGs entropy pool */
        void pushEntropy(const uint8_t *data, size_t length);

        /** Extract random data from the entropy pool */
        void generate(uint8_t *buffer, size_t length);

    private:
        using state_t = chacha20::state_t;

        state_t m_state;
        state_t m_stream;

        void advanceBlock();
    };
}

#endif // UB_CRYPTO_RANDOM_H
