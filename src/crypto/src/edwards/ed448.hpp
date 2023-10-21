#ifndef UB_SRC_CRYPTO_EDWARDS_ED448_H
#define UB_SRC_CRYPTO_EDWARDS_ED448_H

#include "bigint.hpp"
#include "fprime8.hpp"

namespace ub::crypto::impl {
    /** Order of Curve448 elliptic group field, also known as `L` */
    extern const fp8_field_t C448_ORDER;

    /** Ed448 point in projective coordinates */
    class ed448_pt {
    public:
        uint448_t x;
        uint448_t y;
        uint448_t z;

        /**
         * Load a point from compressed representation in buffer. Returns `true` if point is valid.
         * Leaves object in an inconsistent state if input was invalid.
         */
        bool load(const uint8_t *buffer);

        /** Store compressed representation of a point in buffer */
        void store(uint8_t *buffer);

        /** Test whether two points are equal */
        [[nodiscard]] bool equals(const ed448_pt &other) const;

        /** Load neutral point to this object */
        void loadNeutral();

        /** Load base point to this object */
        void loadBase();

        /** Convert point from projected coordinates to affine */
        void unproject();
    };

    namespace ED448 {
        /** Compute `R = A + B` */
        void add(ed448_pt &r, const ed448_pt &a, const ed448_pt &b);

        /** Compute `R = kX`. `R` and `X` must be distinct object. */
        void mul(ed448_pt &r, const ed448_pt &x, const uint448_t &k);
    }
}

#endif // UB_SRC_CRYPTO_EDWARDS_ED448_H
