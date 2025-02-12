#ifndef UB_SRC_CRYPTO_EDWARDS_ED25519_H
#define UB_SRC_CRYPTO_EDWARDS_ED25519_H

#include "f25519.hpp"
#include "fprime8.hpp"

namespace ub::crypto::impl {
    /** Order of Curve25519 elliptic group field, also known as `L` */
    extern const fp8_field_t C25519_ORDER;

    /** Ed25519 point in projective coordinates */
    class ed25519_pt {
    public:
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

        /** Test whether two points are equal */
        [[nodiscard]] bool equals(const ed25519_pt &other) const;

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

#endif // UB_SRC_CRYPTO_EDWARDS_ED25519_H
