#ifndef UB_CRYPTO_AES_H
#define UB_CRYPTO_AES_H

#include <cstdint>
#include <cstddef>

namespace ub::crypto {
    /** Raw (ECB) AES encryption context */
    class aes {
    public:
        /** Create new AES context */
        explicit aes();

        /** Destroy AES context, clearing all sensitive data */
        ~aes() { reset(); }

        /** Reset AES context, clearing all sensitive data */
        void reset();

        /**
         * Initialize AES engine, deriving all round keys.
         *
         * @param key    Key material
         * @param length Key length in bytes
         * @return `true` if context was initialized, `false` if key size is invalid.
         */
        bool init(const uint8_t *key, size_t length);

        /** Encrypt a single data block in-place */
        void encrypt(uint8_t *block);

        /** Length of a single AES block */
        constexpr static uint32_t BLOCK = 16;

    private:
        constexpr static size_t WORD = 4;
        constexpr static size_t MAX_ROUNDS = 14;
        static uint8_t sBox[UINT8_MAX + 1];

        uint8_t Rk[(aes::MAX_ROUNDS + 1) * aes::BLOCK];     //! AES expanded round key
        uint8_t Nr;                                         //! Number of AES rounds

        static void subShift(uint8_t *state);
        static void mixColumns(uint8_t *state);
        static void generateSBox();
        void addRoundKey(uint32_t k, uint8_t *state);
    };

    /** AES CTR encryption and decryption context */
    class aes_ctr {
    public:
        /** Create new uninitialized AES-CTR context */
        explicit aes_ctr();

        /** Destroy AES-CTR context, clearing all sensitive data */
        ~aes_ctr() { reset(); }

        /** Reset AES-CTR context, clearing all sensitive data */
        void reset();

        /** Initialize AES-CTR context */
        bool init(const uint8_t *key, const uint8_t *nonce, size_t keyLength);

        /** Encrypt or decrypt data buffer */
        void process(uint8_t *buffer, size_t length);

    private:
        aes     m_aes;
        uint8_t m_counter[aes::BLOCK];
        uint8_t m_stream[aes::BLOCK];
        uint8_t m_streamPos;
    };
}

#endif // UB_CRYPTO_AES_H
