#ifndef UB_CRYPTO_C25519_H
#define UB_CRYPTO_C25519_H

#include <cstddef>
#include <cstdint>

namespace ub::crypto {
    namespace ed25519 {
        /** Length of Ed25519 private and public keys */
        constexpr static size_t KEY_LENGTH = 32;

        /** Length of Ed25519 signature */
        constexpr static size_t SIGNATURE_LENGTH = 64;

        /** Compute public key from private key */
        void toPublic(uint8_t *publicKey, const uint8_t *privateKey);

        /**
         * Sign a message with Ed25519.
         *
         * @param key       Ed25519 private key buffer (of length `KEY_SIZE`)
         * @param signature Output signature buffer (of length `SIGNATURE_SIZE`)
         * @param message   Message buffer
         * @param length    Length of message
         */
        void sign(const uint8_t *key, uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Sign a SHA-512 hash of message with Ed25519ph.
         *
         * @param key       Ed25519 private key buffer (of length `KEY_SIZE`)
         * @param signature Output signature buffer (of length `SIGNATURE_SIZE`)
         * @param hash      Message hash (of length `ub::crypto::sha512::OUTPUT`)
         */
        void signHash(const uint8_t *key, uint8_t *signature, const uint8_t *hash);

        /**
         * Verify a message signed with Ed25519.
         *
         * @param key       Ed25519 public key buffer (of length `KEY_SIZE`)
         * @param signature Message signature buffer (of length `SIGNATURE_SIZE`)
         * @param message   Message buffer
         * @param length    Length of the message
         * @return          true if signature is valid
         */
        bool verify(const uint8_t *key, const uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Verify a SHA-512 hash of message with Ed25519ph.
         *
         * @param key       Ed25519 public key buffer (of length `KEY_SIZE`)
         * @param signature Message signature buffer (of length `SIGNATURE_SIZE`)
         * @param hash      Message hash (of length `ub::crypto::sha512::OUTPUT`)
         * @return          true if signature is valid
         */
        bool verifyHash(const uint8_t *key, const uint8_t *signature, const uint8_t *hash);
    }
}

#endif // UB_CRYPTO_C25519_H
