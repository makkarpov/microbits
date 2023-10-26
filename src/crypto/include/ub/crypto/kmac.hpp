#ifndef UB_CRYPTO_KMAC_H
#define UB_CRYPTO_KMAC_H

#include <cstdint>
#include <cstddef>

#include <ub/crypto/sha3.hpp>

namespace ub::crypto {
    class kmac {
    public:
        /** Create new KMAC instance */
        explicit kmac();

        /** Destroy KMAC instance, erasing all sensitive data */
        ~kmac() { reset(); }

        /** Reset KMAC instance to it's default state, erasing all sensitive data */
        void reset() {
            k.reset();
            m_macLength = 0;
        }

        /**
         * Initialize KMAC instance and prepare it to consume data.
         *
         * Variant is specified as either KMAC_128 or KMAC_256 static constant.
         * Key and MAC lengths are specified in bytes.
         */
        void init(uint32_t variant, const uint8_t *key, size_t keyLength, size_t macLength);

        /** Update KMAC instance with additional data. */
        void update(const uint8_t *data, size_t length) { k.consume(data, length); }

        /** Finish MAC operation and generate final MAC value */
        void finish(uint8_t *mac);

        constexpr static uint32_t KMAC_128 = 0;
        constexpr static uint32_t KMAC_256 = 1;

    private:
        keccak1600  k;
        size_t      m_macLength;
    };
}

#endif // UB_CRYPTO_KMAC_H
