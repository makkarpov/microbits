#ifndef UB_CRYPTO_SHA2_H
#define UB_CRYPTO_SHA2_H

#include <ub/crypto/utility.hpp>

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace ub::crypto {
    class sha256 {
    public:
        /** Create new empty SHA-256 context */
        explicit sha256();

        /** Destroy SHA-256 context, clearing all sensitive data */
        ~sha256() { reset(); }

        /** Reset SHA-256 context to prepare for new hashing, clearing all sensitive data */
        void reset();

        /** Update SHA-256 state with input data */
        void update(const uint8_t *data, size_t length);

        /** Finish hashing operation, producing the final digest */
        void finish(uint8_t *digest);

        /** Length of SHA-256 internal block */
        constexpr static size_t BLOCK = 64;

        /** Length of SHA-256 output digest */
        constexpr static size_t OUTPUT = 32;

    private:
        static constexpr uint32_t W_STATE = OUTPUT / sizeof(uint32_t);
        static constexpr uint32_t W_BLOCK = BLOCK / sizeof(uint32_t);
        static constexpr uint32_t W_ROUND = 64;

        static const uint32_t initialState[W_STATE];
        static const uint32_t roundConstants[W_ROUND];

        uint8_t  m_block[BLOCK];
        uint32_t m_state[W_STATE];
        uint32_t m_totalBytes;

        void processBlock();
    };

    class sha512 {
    public:
        /** Create new empty SHA-512 context */
        explicit sha512();

        /** Destroy SHA-512 context, clearing all sensitive data */
        ~sha512() { reset(); }

        /** Reset SHA-512 context to prepare for new hashing, clearing all sensitive data */
        void reset();

        /** Update SHA-512 state with input data */
        void update(const uint8_t *data, size_t length);

        /** Finish hashing operation, producing the final digest */
        void finish(uint8_t *digest);

        /** Length of SHA-512 internal block */
        constexpr static size_t BLOCK = 128;

        /** Length of SHA-512 output digest */
        constexpr static size_t OUTPUT = 64;

    private:
        static constexpr uint32_t W_STATE = OUTPUT / sizeof(uint64_t);
        static constexpr uint32_t W_BLOCK = BLOCK / sizeof(uint64_t);
        static constexpr uint32_t W_ROUND = 80;

        static const uint64_t initialState[W_STATE];
        static const uint64_t roundConstants[W_ROUND];

        uint8_t  m_block[BLOCK];
        uint64_t m_state[W_STATE];
        size_t   m_totalBytes;

        void processBlock();
    };
}

#endif // UB_CRYPTO_SHA2_H
