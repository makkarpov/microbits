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
    namespace F25519 {
        /** Compute `x = x % p` */
        void normalize(uint256_t &x);

        /** Compute `r = a + b` */
        void add(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = v1 - v2` */
        void sub(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = p - x` */
        void neg(uint256_t &r, const uint256_t &x);

        /** Compute `r = a * b`. `r` must not point to `a` or `b` */
        void mul(uint256_t &r, const uint256_t &a, const uint256_t &b);

        /** Compute `r = x^-1 mod p`. `r` must not point to `x` */
        void inv(uint256_t &r, const uint256_t &x);

        /** Compute `r = x^((p-5)/8) mod p` */
        void pow58(uint256_t &r, const uint256_t &x);

        /** Compute `r = sqrt(x) mod p`. */
        void sqrt(uint256_t &r, const uint256_t &x);
    }
}

#endif // UB_SRC_CRYPTO_EDWARDS_F25519_H
