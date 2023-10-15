#ifndef UB_CRYPTO_SHA3_H
#define UB_CRYPTO_SHA3_H

#include <cstddef>
#include <cstdint>
#include <array>

namespace ub::crypto {
    /** Raw Keccak 1600 state update function */
    namespace keccak1600 {
        /** Length of the intermediate state in bytes */
        constexpr static size_t LENGTH = 200;

        /** Internal state array typedef for convenience */
        using state_t = std::array<uint8_t, LENGTH>;

        /** Apply Keccak block permutation to the state array */
        void apply(uint8_t *state);

        /** Apply Keccak block permutation to the state array */
        inline void apply(state_t &state) { apply(state.data()); }
    }

    /** SHA-3 instance with arbitrary digest length */
    class sha3 {
    public:
        /**
         * Create new empty SHA-3 context. Digest length must be provided either in constructor or via `reset()` method
         * before actual hashing starts.
         */
        explicit sha3(uint32_t digestLength = 0);

        /** Destroy SHA-3 context, clearing all sensitive data */
        ~sha3() { reset(); }

        /**
         * Reset SHA-3 context to prepare for new hashing, clearing all sensitive data.
         * Changes digest length if length is specified. Digest length is specified in bytes.
         */
        void reset(size_t digestLength = 0);

        /** Update SHA-3 state with input data */
        void update(const uint8_t *data, size_t length);

        /** Finish hashing operation, producing the final digest */
        void finish(uint8_t *digest);

        /** Digest length for SHA3-224 variant */
        constexpr static uint32_t DIGEST_224 = 28;

        /** Digest length for SHA3-256 variant */
        constexpr static uint32_t DIGEST_256 = 32;

        /** Digest length for SHA3-384 variant */
        constexpr static uint32_t DIGEST_384 = 48;

        /** Digest length for SHA3-512 variant */
        constexpr static uint32_t DIGEST_512 = 64;

    private:
        using kstate_t = keccak1600::state_t;

        kstate_t m_state;
        uint8_t  m_digestLen;
        uint8_t  m_ptr;
    };

    /** SHAKE extensible output function instance */
    class shake {
    public:
        /**
         * Create new empty SHAKE context. Function variant must be initialized either in constructor of via `reset()`
         * method before actual processing starts.
         */
        explicit shake(uint32_t variant = 0);

        /** Destroy SHAKE context, clearing all sensitive data */
        ~shake() { reset(); }

        /**
         * Reset SHAKE context to prepare for new hashing, clearing all sensitive data.
         * Changes function variant if specified.
         */
        void reset(uint32_t variant = 0);

        /** Update SHAKE state with new data */
        void update(const uint8_t *data, size_t length);

        /** Generate a next block of SHAKE function output */
        void generate(uint8_t *output, size_t length);

        /** Function variant for SHAKE128 */
        static constexpr uint32_t FN_SHAKE128 = 1;

        /** Function variant for SHAKE256 */
        static constexpr uint32_t FN_SHAKE256 = 2;

    private:
        using kstate_t = keccak1600::state_t;

        kstate_t m_state;
        uint8_t  m_variant;
        uint8_t  m_ptr;
        bool     m_generating;

        void process(uint8_t *buffer, size_t length, bool consume);
    };
}

#endif // UB_CRYPTO_SHA3_H
