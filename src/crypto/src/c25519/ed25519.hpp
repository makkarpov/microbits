#ifndef UB_SRC_CRYPTO_C25519_ED25519_H
#define UB_SRC_CRYPTO_C25519_ED25519_H

#include "f25519.hpp"

namespace ub::crypto::impl {
    /** Ed25519 point in projective coordinates */
    struct ed25519_pt {
        uint256_t x;
        uint256_t y;
        uint256_t t;
        uint256_t z;

        /** Convert a point from affine coordinates (stored in x & y) to projected representation */
        void project();

        /**
         * Load a point from compressed representation in buffer. Returns `true` if point is valid.
         * Leaves object in an inconsistent state if input was invalid.
         */
        bool load(const uint8_t *buffer);

        /** Convert a point from projected representation to affine coordinates */
        void unproject();

        /** Store a compressed representation of this point in buffer */
        void store(uint8_t *buffer);

        /** Load neutral point */
        void loadNeutral();

        /** Load base point */
        void loadBase();

        /** Securely erase contents of this point */
        void destroy();
    };

    static_assert(std::is_trivially_copyable_v<ed25519_pt>, "ed25519_pt must be trivially copyable");
    static_assert(std::is_trivially_destructible_v<ed25519_pt>, "ed25519_pt must be trivially destructible");

    namespace ED25519 {
        /** Compute `R = X + Y` */
        void add(ed25519_pt &r, const ed25519_pt &x, const ed25519_pt &y);

        /** Compute `R = kX`. `R` and `X` must be distinct objects. */
        void mul(ed25519_pt &r, const ed25519_pt &x, const uint256_t &k);
    }
}

#endif // UB_SRC_CRYPTO_C25519_ED25519_H
