#ifndef UB_UTILITIES_ERRORS_H
#define UB_UTILITIES_ERRORS_H

#include <cstdint>

namespace ub {
    /** A lightweight version for `std::error_category`, keeping only strictly necessary elements. */
    class ErrorCategory {
    public:
        /** Default constructor for error category. */
        constexpr ErrorCategory() = default;

        /** Error categories cannot be copied. */
        ErrorCategory(const ErrorCategory &) = delete;

        /** Error categories cannot be moved because they are referenced by absolute address. */
        ErrorCategory(ErrorCategory &&) = delete;

        /** Error categories cannot be copy-assigned. */
        ErrorCategory &operator=(const ErrorCategory &) = delete;

        /** Error categories cannot be move-assigned */
        ErrorCategory &operator=(ErrorCategory &&) = delete;

        /**
         * @return Name of this category. {@code nullptr} could be returned if name is not available or is stripped down
         *         to reduce binary size.
         */
        [[nodiscard]] virtual const char *categoryName() const { return nullptr; }

        /**
         * @param code Error code within this category.
         * @return Name of given error code. {@code nullptr} could be returned if name is not available or is stripped
         *         down to reduce binary size.
         */
        [[nodiscard]] virtual const char *errorName(uint32_t code) const { return nullptr; }
    };

    /** Status code structure. */
    struct Status {
        /** Category of active error. {@code nullptr} if no error has been recorded in this status structure. */
        const ErrorCategory     *category;

        /** Error code (interpretation is specific to category) of active error. */
        uint32_t                error_code;

        /** Initialize structure with successful state. */
        inline explicit Status() noexcept: category(nullptr), error_code(0) {}

        /** @return Whether this status structure represents successful completion */
        inline explicit operator bool() const noexcept { return category == nullptr; }

        /**
         * Assign an error to this status code, if no errors have been reported already.
         *
         * @param cat  Error category to assign
         * @param code Code within a category
         */
        void setError(const ErrorCategory &cat, uint32_t code) noexcept;

        /** Clear current error condition. */
        void clearError() noexcept;
    };

    extern const ErrorCategory genericError;

    enum {
        E_TIMEOUT           = 0,    //! Timeout has occurred during operation
        E_OUT_OF_MEMORY     = 1,    //! Application has run out of heap
        E_INVALID_ARGUMENT  = 2,    //! Argument validation failed
        E_INVALID_STATE     = 3,    //! Operation is not possible in current state
    };
}

#define STATUS_CODE                 _statusCode
#define STATUS_CODE_DEFN            ub::Status STATUS_CODE
#define HAS_PENDING_ERROR           (!STATUS_CODE)

/**
 * Use this macros at the end of function argument list to indicate that function might throw an error:
 *
 * void foo(int a, int b, THROWS);
 */
#define THROWS                      ub::Status &STATUS_CODE

/**
 * Use these macros when invoking throwing function and when you want to delegate this error to upper level:
 *
 * int bar(THROWS) {
 *   foo(123, 456, CHECK_0);
 * }
 */
#define CHECK                       STATUS_CODE); if (HAS_PENDING_ERROR) return;        (void) (0
#define CHECK_(result)              STATUS_CODE); if (HAS_PENDING_ERROR) return result; (void) (0
#define CHECK_0                     CHECK_(0)
#define CHECK_NULL                  CHECK_(nullptr)
#define CHECK_FALSE                 CHECK_(false)

/**
 * Use these macros when throwing an error out of function:
 *
 * bool foo(int a, int b, THROWS) {
 *   if (a < 0) THROW_FALSE(fooCategory, FOO_ERROR_NEGATIVE_VALUE);
 * }
 */
#define THROW(category, code)       STATUS_CODE.setError(category, code); return
#define THROW_(category, code, r)   STATUS_CODE.setError(category, code); return r
#define THROW_0(category, code)     THROW_(category, code, 0)
#define THROW_NULL(category, code)  THROW_(category, code, nullptr)
#define THROW_FALSE(category, code) THROW_(category, code, false)

#endif // UB_UTILITIES_ERRORS_H
