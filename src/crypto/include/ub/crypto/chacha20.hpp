#ifndef UB_CRYPTO_CHACHA20_H
#define UB_CRYPTO_CHACHA20_H

#include <cstdint>
#include <cstddef>

namespace ub::crypto {
    class chacha20 {
    public:
        /** Length of encryption key in bytes */
        constexpr static size_t KEY_LENGTH = 32;

        /** Length of nonce in bytes */
        constexpr static size_t NONCE_LENGTH = 12;

        /** Initialize ChaCha20 instance */
        explicit chacha20();

        /** Destroy ChaCha20 instance, erasing sensitive data */
        ~chacha20();

        /** Initialize cipher with key and nonce */
        void init(const uint8_t *key, const uint8_t *nonce);

        /** Initialize cipher with key and nonce as unsigned integer */
        void init(const uint8_t *key, uint64_t nonce);

        /** Process (encrypt or decrypt) a buffer. This function could operate in-place. */
        void process(uint8_t *dst, const uint8_t *src, size_t length);

        // Low-level functions to use ChaCha20 primitive in other cryptographic constructs:

        /** Process raw ChaCha20 state block located in `src`, placing the result to `dst` */
        static void processBlock(uint32_t *dst, const uint32_t *src);

        /** Raw ChaCha20 state container type */
        union state_t {
            constexpr static size_t LENGTH = 64;

            uint8_t  u8[LENGTH];        //! ChaCha20 state viewed as bytes
            uint32_t u32[LENGTH / 4];   //! ChaCha20 state viewed as words
        };

        /** ChaCha20 'expand 32-byte k' constant as an array of little-endian integers */
        static const uint32_t initialConstants[4];

    private:
        state_t  m_state;
        state_t  m_stream;
        uint8_t  m_streamPtr;

        void processBlock();
    };
}

#endif // UB_CRYPTO_CHACHA20_H
