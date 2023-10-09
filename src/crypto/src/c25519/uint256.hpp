#ifndef UB_SRC_CRYPTO_C25519_UINT256_H
#define UB_SRC_CRYPTO_C25519_UINT256_H

#include <ub/crypto/utility.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace ub::crypto::impl {
    /**
     * Cryptographically oriented 256-bit number type. Implements secure (constant time) operations.
     * Stored as little-endian array of components, available as 8, 16 or 32-bit numbers.
     */
    class uint256_t {
    public:
        constexpr static size_t N_U32 = 8;
        constexpr static size_t N_U16 = 16;
        constexpr static size_t N_U8  = 32;

        struct from_u8_t {};
        constexpr static from_u8_t from_u8 {};

        union {
            uint8_t  u8[N_U8];      //! Number represented as bytes
            uint16_t u16[N_U16];    //! Number represented as shorts
            uint32_t u32[N_U32];    //! Number represented as words
        };

        /** Constructor to initialize number to zero */
        constexpr uint256_t() noexcept: u32 {} {}

        /** Constructor to initialize number to 32-bit value */
        explicit constexpr uint256_t(uint32_t v) noexcept: u32 { v } {}

        /** Constructor to initialize number from constant 8-bit data array */
        template <typename ...T>
        explicit constexpr uint256_t(from_u8_t, T&& ...data) noexcept: u8 { (uint8_t) std::forward<T>(data)... } {}

        /** Trivial copy constructor */
        constexpr uint256_t(const uint256_t &) noexcept = default;

        /** @return Whether two integers are bitwise equal */
        inline bool operator==(const uint256_t &other) const { return secureCompare(u32, other.u32, sizeof(u32)); }

        /** @return Whether two integers are bitwise different */
        inline bool operator!=(const uint256_t &other) const { return !secureCompare(u32, other.u32, sizeof(u32)); }

        /** Load a 32-bit integer into this number object */
        uint256_t &operator=(uint32_t v);

        /** Copy another number into this object */
        uint256_t &operator=(const uint256_t &x) = default;

        /** If condition is false, v_false is copied to this number object. Otherwise, v_true is copied. */
        void select(bool condition, const uint256_t &v_false, const uint256_t &v_true);

        /**
         * Securely erase contents of this number.
         *
         * Unfortunately, a destructor cannot be used:
         *  * it breaks 'trivially copyable' property
         *  * it prevents creation of constexpr values
         */
        void destroy();
    };

    /** Required for certain operations where hash output is interpreted as int */
    static_assert(std::is_trivially_copyable_v<uint256_t>, "uint256_t must be trivially copyable");
}

#endif // UB_SRC_CRYPTO_C25519_UINT256_H
