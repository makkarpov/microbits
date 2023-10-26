#ifndef UB_CRYPTO_HMAC_H
#define UB_CRYPTO_HMAC_H

#include <cstdint>
#include <cstddef>

namespace ub::crypto {
    /** Generic HMAC primitive over arbitrary hash function */
    template <typename hash>
    class hmac {
    public:
        /** Create new empty HMAC context */
        explicit hmac(): m_key {} {}

        /** Destroy HMAC context, clearing all sensitive data */
        ~hmac() { reset(); }

        /** Reset HMAC context, clearing all sensitive data */
        void reset() {
            m_hash.reset();
            secureZero(m_key, sizeof(m_key));
        }

        /** Initialize HMAC context with key and prepare it to consume data */
        void init(const uint8_t *key, size_t length) {
            memset(m_key, 0, sizeof(m_key));

            if (length > hash::BLOCK) {
                m_hash.reset();
                m_hash.update(key, length);
                m_hash.finish(m_key);
            } else {
                memcpy(m_key, key, length);
            }

            maskKey(I_PAD);

            m_hash.reset();
            m_hash.update(m_key, hash::BLOCK);
        }

        /** Update HMAC context with data */
        void update(const uint8_t *data, size_t length) {
            m_hash.update(data, length);
        }

        /** Finish HMAC computation */
        void finish(uint8_t *mac) {
            m_hash.finish(mac);
            maskKey(I_PAD ^ O_PAD); // undo I_PAD and apply O_PAD

            m_hash.reset();
            m_hash.update(m_key, hash::BLOCK);
            m_hash.update(mac, hash::OUTPUT);
            m_hash.finish(mac);
        }

        /** HMAC output length */
        constexpr static uint32_t OUTPUT = hash::OUTPUT;

    private:
        constexpr static uint8_t O_PAD = 0x5C;
        constexpr static uint8_t I_PAD = 0x36;

        hash    m_hash;
        uint8_t m_key[hash::BLOCK];

        /** XOR all key bytes with padding byte */
        void maskKey(uint8_t pad) {
            for (uint8_t &k: m_key) {
                k ^= pad;
            }
        }
    };
}

#endif // UB_CRYPTO_HMAC_H
