#ifndef UB_SRC_CRYPTO_EDWARDS_F25519_H
#define UB_SRC_CRYPTO_EDWARDS_F25519_H

#include <cstdint>
#include <cstddef>

#include <array>

#include <ub/crypto/utility.hpp>
#include <cstring>

#include "bigint.hpp"

namespace ub::crypto::impl {
    /** Operations specific to `Fp(2**255 - 19)` */
    struct F25519 {
        /** Underlying big integer type */
        using uint_t = uint256_t;

        /** Compute `x = x % p` */
        static void normalize(uint256_t &x);

        /** Compute `r = a + b` */
        static void add(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = v1 - v2` */
        static void sub(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = p - x` */
        static void neg(uint256_t &r, const uint256_t &x);

        /** Compute `r = a * b`. `r` must not point to `a` or `b` */
        static void mul(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = x^-1 mod p`. `r` must not point to `x` */
        static void inv(uint256_t &r, const uint256_t &x);

        /** Compute `r = x^((p-5)/8) mod p` */
        static void pow58(uint256_t &r, const uint256_t &x);
    };
}

#endif // UB_SRC_CRYPTO_EDWARDS_F25519_H
