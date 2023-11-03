#ifndef UB_USB_DEVICE_BASE_USBD_ERRORS_H
#define UB_USB_DEVICE_BASE_USBD_ERRORS_H

#include <ub/errors.hpp>

namespace ub::usbd {
    enum {
        E_INVALID_SETUP_LENGTH          = 1,
        E_UNRESOLVED_CONTROL_REQUEST    = 2,
        E_CONTROL_REQUEST_REJECTED      = 3,
        E_CONTROL_VALIDATION_FAILED     = 4,
        E_CONTROL_DATA_TOO_LONG         = 5,
        E_FUNCTION_MISMATCH             = 6,
    };

    extern const ErrorCategory usbError;
}

#endif // UB_USB_DEVICE_BASE_USBD_ERRORS_H
