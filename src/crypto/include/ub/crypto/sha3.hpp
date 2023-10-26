#ifndef UB_CRYPTO_SHA3_H
#define UB_CRYPTO_SHA3_H

#include <ub/crypto/utility.hpp>

#include <cstddef>
#include <cstdint>
#include <array>

namespace ub::crypto {
    /** Raw Keccak 1600 state update function and streaming primitive */
    struct keccak1600 {
        // Static declarations: ----------------------------------------------------------------------------------------

        /** Length of the intermediate state in bytes */
        constexpr static size_t LENGTH = 200;

        /** Internal state array type */
        union state_t {
            uint8_t  u8[LENGTH];
            uint32_t u32[LENGTH / sizeof(uint32_t)];
        };

        /** Apply Keccak block permutation to the state array */
        static void apply(state_t &state);

        // Streaming Keccak1600 primitive: -----------------------------------------------------------------------------

        state_t st;     // Current keccak state
        uint8_t ptr;    // Pointer to write next byte into state
        uint8_t rate;   // Rate of this function in bytes

        /** Reset this object to empty state */
        void reset() {
            secureZero(st.u8, LENGTH);
            ptr = 0;
        }

        /** Absorb more data into this Keccak instance */
        void consume(const uint8_t *buf, size_t length);

        /** Finalize Keccak operation and prepare for data generation */
        void finish(uint8_t trailer);

        /** Produce next chunk of data from Keccak instance */
        void produce(uint8_t *buf, size_t length);
    };

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
        void update(const uint8_t *data, size_t length) { k.consume(data, length); }

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
        keccak1600  k;
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
        keccak1600  k;
        bool        m_generating;
    };
}

#endif // UB_CRYPTO_SHA3_H
