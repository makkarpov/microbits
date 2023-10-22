#ifndef UB_SRC_CRYPTO_EDWARDS_BIGINT_H
#define UB_SRC_CRYPTO_EDWARDS_BIGINT_H

#include <ub/crypto/utility.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

namespace ub::crypto::impl {
    /**
     * Cryptographically oriented big number container type. Implements secure (constant time) operations that are
     * independent of underlying field (i.e. equality comparison or constant time select() operator).
     * Stored as little-endian array of components, available as 8, 16 or 32-bit numbers.
     */
    template <size_t N_bits>
    class bigint_t {
    public:
        constexpr static size_t N_BITS = N_bits;
        constexpr static size_t N_U32  = N_bits / 32;
        constexpr static size_t N_U16  = N_bits / 16;
        constexpr static size_t N_U8   = N_bits / 8;

        struct from_u8_t {};
        constexpr static from_u8_t from_u8 {};

        union {
            uint8_t  u8 [N_U8];     //! Number represented as bytes
            uint16_t u16[N_U16];    //! Number represented as shorts
            uint32_t u32[N_U32];    //! Number represented as words
        };

        /** Constructor to initialize number to zero */
        constexpr bigint_t() noexcept: u32 {} {}

        /** Constructor to initialize number to 32-bit value */
        explicit constexpr bigint_t(uint32_t v) noexcept: u32 { v } {}

        /** Constructor to initialize number from constant 8-bit data array */
        template <typename ...T>
        explicit constexpr bigint_t(from_u8_t, T&& ...data) noexcept: u8 { (uint8_t) std::forward<T>(data)... } {}

        /** Trivial copy constructor */
        constexpr bigint_t(const bigint_t<N_bits> &) noexcept = default;

        /** @return Whether two integers are bitwise equal */
        inline bool operator==(const bigint_t &other) const { return secureCompare(u32, other.u32, sizeof(u32)); }

        /** @return Whether two integers are bitwise different */
        inline bool operator!=(const bigint_t &other) const { return !secureCompare(u32, other.u32, sizeof(u32)); }

        /** Load a 32-bit integer into this number object */
        bigint_t &operator=(uint32_t v) {
            std::memset(u8, 0, N_U8);
            u32[0] = v;
            return *this;
        }

        /** Copy another number into this object */
        bigint_t &operator=(const bigint_t &x) = default;

        /** If condition is false, v_false is copied to this number object. Otherwise, v_true is copied. */
        void select(bool condition, const bigint_t &v_false, const bigint_t &v_true) {
            uint32_t mask = -(condition & 1);   // 0 -> 0x00, 1 -> 0xFF

            for (size_t i = 0; i < N_U32; i++) {
                uint32_t diff = v_false.u32[i] ^ v_true.u32[i];
                u32[i] = v_false.u32[i] ^ (diff & mask);
            }
        }

        /**
         * Securely erase contents of this number.
         *
         * Unfortunately, a destructor cannot be used:
         *  * it breaks 'trivially copyable' property
         *  * it prevents creation of constexpr values
         */
        void destroy() { secureZero(u8, N_U8); }

        /** If condition is false, leaves `u` and `v` unmodified. Otherwise, swaps `u` and `v`. */
        static void swap(bool condition, bigint_t &u, bigint_t &v) {
            uint32_t mask = -(condition & 1);   // 0 -> 0x00, 1 -> 0xFF

            for (size_t i = 0; i < N_U32; i++) {
                uint32_t diff = (u.u32[i] ^ v.u32[i]) & mask;
                u.u32[i] = u.u32[i] ^ diff;
                v.u32[i] = v.u32[i] ^ diff;
            }
        }
    };

    using uint256_t = bigint_t<256>;
    using uint448_t = bigint_t<448>;

    /** Required for certain operations where hash output is interpreted as int */
    static_assert(std::is_trivially_copyable_v<uint256_t>, "uint256_t must be trivially copyable");

    /**
     * Perform modular exponentiation with a constant exponent. Exponent is specified in compressed binary form,
     * starting from most significant bit with first most significant '1' bit dropped. LSB of exponent entry encodes
     * actual bit value (0 or 1), MSBs encode repetition count. Exponent must be terminated with null byte.
     *
     * Depending on the number of swaps (which is determined by number of zero bits in exponent) final result might be
     * either in `r` or in `s`. If exponent has even number of zero bits, result is in `r`. Otherwise, result is in `s`.
     */
    template <typename F, typename uint_t = typename F::uint_t>
    void bigint_pow_rle(uint_t &r, uint_t &s, const uint_t &x, const uint8_t *exponent) {
        uint_t *a = &s;
        uint_t *b = &r;

        std::memcpy(r.u8, x.u8, sizeof(r.u8));

        uint8_t i;
        while ((i = *exponent) != 0) {
            while (i > 1) {
                F::mul(*a, *b, *b);

                if (i & 1) {
                    F::mul(*b, *a, x);
                } else {
                    std::swap(a, b);
                }

                i -= 2;
            }

            exponent++;
        }
    }
}

#endif // UB_SRC_CRYPTO_EDWARDS_BIGINT_H
