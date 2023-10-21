#ifndef UB_CRYPTO_EDWARDS_H
#define UB_CRYPTO_EDWARDS_H

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
         * @param key       Buffer with concatenation of Ed25519 private key with it's matching public key (giving total
         *                  length of `2*KEYSIZE` bytes). Use `toPublic` function to derive public key from private.
         * @param signature Output signature buffer (of length `SIGNATURE_LENGTH`)
         * @param message   Message buffer
         * @param length    Length of message
         */
        void sign(const uint8_t *key, uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Sign a SHA-512 hash of message with Ed25519ph.
         *
         * @param key       Buffer with concatenation of Ed25519 private key with it's matching public key (giving total
         *                  length of `2*KEYSIZE` bytes). Use `toPublic` function to derive public key from private.
         * @param signature Output signature buffer (of length `SIGNATURE_LENGTH`)
         * @param hash      Message hash (of length `ub::crypto::sha512::OUTPUT`)
         */
        void signHash(const uint8_t *key, uint8_t *signature, const uint8_t *hash);

        /**
         * Verify a message signed with Ed25519.
         *
         * @param key       Ed25519 public key buffer (of length `KEY_LENGTH`)
         * @param signature Message signature buffer (of length `SIGNATURE_LENGTH`)
         * @param message   Message buffer
         * @param length    Length of the message
         * @return          true if signature is valid
         */
        bool verify(const uint8_t *key, const uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Verify a SHA-512 hash of message with Ed25519ph.
         *
         * @param key       Ed25519 public key buffer (of length `KEY_LENGTH`)
         * @param signature Message signature buffer (of length `SIGNATURE_LENGTH`)
         * @param hash      Message hash (of length `ub::crypto::sha512::OUTPUT`)
         * @return          true if signature is valid
         */
        bool verifyHash(const uint8_t *key, const uint8_t *signature, const uint8_t *hash);
    }

    namespace ed448 {
        /** Length of Ed448 private and public keys */
        constexpr static size_t KEY_LENGTH = 57;

        /** Length of Ed448 signature */
        constexpr static size_t SIGNATURE_LENGTH = 114;

        /** Length of input SHAKE256 hash for xxxHash variants */
        constexpr static size_t HASH_LENGTH = 64;

        /** Compute public key from private key */
        void toPublic(uint8_t *publicKey, const uint8_t *privateKey);

        /**
         * Sign a message with Ed448.
         *
         * @param key       Buffer with concatenation of Ed448 private key with it's matching public key (giving total
         *                  length of `2*KEYSIZE` bytes). Use `toPublic` function to derive public key from private.
         * @param signature Output signature buffer (of length `SIGNATURE_LENGTH`)
         * @param message   Message buffer
         * @param length    Length of message
         */
        void sign(const uint8_t *key, uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Sign a SHAKE-256[64] hash of message with Ed448ph.
         *
         * @param key       Buffer with concatenation of Ed448 private key with it's matching public key (giving total
         *                  length of `2*KEYSIZE` bytes). Use `toPublic` function to derive public key from private.
         * @param signature Output signature buffer (of length `SIGNATURE_LENGTH`)
         * @param hash      Message hash (of length `HASH_LENGTH`)
         */
        void signHash(const uint8_t *key, uint8_t *signature, const uint8_t *hash);

        /**
         * Verify a message signed with Ed448.
         *
         * @param key       Ed448 public key buffer (of length `KEY_LENGTH`)
         * @param signature Message signature buffer (of length `SIGNATURE_LENGTH`)
         * @param message   Message buffer
         * @param length    Length of the message
         * @return          true if signature is valid
         */
        bool verify(const uint8_t *key, const uint8_t *signature, const uint8_t *message, size_t length);

        /**
         * Verify a SHAKE-256[64] hash of message with Ed448ph.
         *
         * @param key       Ed448 public key buffer (of length `KEY_LENGTH`)
         * @param signature Message signature buffer (of length `SIGNATURE_LENGTH`)
         * @param hash      Message hash (of length `HASH_LENGTH`)
         * @return          true if signature is valid
         */
        bool verifyHash(const uint8_t *key, const uint8_t *signature, const uint8_t *hash);
    }
}

#endif // UB_CRYPTO_EDWARDS_H
