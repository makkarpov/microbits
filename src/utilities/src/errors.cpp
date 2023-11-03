#include <ub/errors.hpp>

const ub::ErrorCategory ub::genericError;

void ub::Status::setError(const ub::ErrorCategory &cat, uint32_t code) noexcept {
    if (this->category != nullptr) {
        return;
    }

    this->category   = &cat;
    this->error_code = code;
}

void ub::Status::clearError() noexcept {
    category    = nullptr;
    error_code  = 0;
}
