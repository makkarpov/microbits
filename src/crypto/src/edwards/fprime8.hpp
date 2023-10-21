#ifndef UB_SRC_CRYPTO_EDWARDS_FPRIME8_H
#define UB_SRC_CRYPTO_EDWARDS_FPRIME8_H

#include "f25519.hpp"

namespace ub::crypto::impl {
    struct fp8_field_t {
        size_t  len;         //! Modulus length in bytes
        const uint8_t *mod;  //! Modulus data
    };

    /** All function assume that modulus is less than 2^255 */
    namespace Fp8 {
        /** Load a big number into the uint256_t object, reducing it modulo m. */
        void load(uint8_t *r, const uint8_t *data, size_t length, const fp8_field_t &f);

        /** Compute `r = (r + a) mod m` */
        void add(uint8_t *r, const uint8_t *a, const fp8_field_t &f);

        /** Compute `r = (a * b) mod m`. `r` must be a distinct object from `a` and `b` */
        void mul(uint8_t *r, const uint8_t *a, const uint8_t *b, const fp8_field_t &f);
    }
}

#endif // UB_SRC_CRYPTO_EDWARDS_FPRIME8_H
