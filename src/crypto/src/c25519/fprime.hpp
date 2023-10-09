#ifndef UB_SRC_CRYPTO_C25519_FPRIME_H
#define UB_SRC_CRYPTO_C25519_FPRIME_H

#include "f25519.hpp"

namespace ub::crypto::impl {
    /** All function assume that modulus is less than 2^255 */
    namespace Fp {
        /** Load a big number into the uint256_t object, reducing it modulo m. */
        void load(uint256_t &r, const uint8_t *data, size_t length, const uint256_t &m);

        /** Compute `r = (r + a) mod m` */
        void add(uint256_t &r, const uint256_t &a, const uint256_t &m);

        /** Compute `r = (a * b) mod m`. `r` must be a distinct object from `a` and `b` */
        void mul(uint256_t &r, const uint256_t &a, const uint256_t &b, const uint256_t &m);
    }
}

#endif // UB_SRC_CRYPTO_C25519_FPRIME_H
