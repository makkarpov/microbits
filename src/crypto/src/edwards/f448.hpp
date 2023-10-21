#ifndef UB_SRC_CRYPTO_EDWARDS_F448_H
#define UB_SRC_CRYPTO_EDWARDS_F448_H

#include "bigint.hpp"

namespace ub::crypto::impl {
    /** Operations specific to `Fp(2**448 - 2**224 - 1)` */
    struct F448 {
        /** Compute `r = x mod p`. Returns true if `x` was greater than `p`. */
        static bool normalize(uint448_t &r, const uint448_t &x);

        /** Compute `r = r mod p`. Returns true if `x` was greater than `p`. */
        static inline bool normalize(uint448_t &r) {
            return normalize(r, r);
        }

        /** Load signed integer into `r`, subtracting it from `p` if necessary */
        static void load(uint448_t &r, int32_t x);

        /** Compute `r = a + b` */
        static void add(uint448_t &r, const uint448_t &a, const uint448_t &b);

        /** Compute `r = p - x` */
        static void neg(uint448_t &r, const uint448_t &x);

        /** Compute `r = a * b`. `r` must be distinct from `a` and `b`. */
        static void mul(uint448_t &r, const uint448_t &a, const uint448_t &b);

        /** Compute `r = x^-1` */
        static void inv(uint448_t &r, const uint448_t &x);

        /** Compute `r = x ^ ((p-3)/4)` */
        static void powP34(uint448_t &r, const uint448_t &x);
    };
}

#endif // UB_SRC_CRYPTO_EDWARDS_F448_H
