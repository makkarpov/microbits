#ifndef UB_USB_DEVICE_BASE_USBD_CONFIG_H
#define UB_USB_DEVICE_BASE_USBD_CONFIG_H

#include <ub/user-config.h>

/** Maximum number of supported IN (device -> host) endpoints per device instance, including the control endpoint */
#if !defined(UB_USBD_MAX_IN_ENDPOINTS)
#define UB_USBD_MAX_IN_ENDPOINTS                                4
#endif

/** Maximum number of supported OUT (host -> device) endpoints per device instance, including the control endpoint */
#if !defined(UB_USBD_MAX_OUT_ENDPOINTS)
#define UB_USBD_MAX_OUT_ENDPOINTS                               4
#endif

/** Maximum number of registered functions per device instance */
#if !defined(UB_USBD_MAX_FUNCTIONS)
#define UB_USBD_MAX_FUNCTIONS                                   1
#endif

/** Maximum number of interfaces per device */
#if !defined(UB_USBD_MAX_INTERFACES)
#define UB_USBD_MAX_INTERFACES                                  4
#endif

/** Maximum number of logical endpoints allowed per function */
#if !defined(UB_USBD_MAX_FUNC_ENDPOINTS)
#define UB_USBD_MAX_FUNC_ENDPOINTS                              4
#endif

/** Maximum allowed length of a control packet */
#if !defined(UB_USBD_MAX_CONTROL_PACKET)
#define UB_USBD_MAX_CONTROL_PACKET                              64
#endif

/** Whether to include support for runtime-generated serial number string descriptors */
#if !defined(UB_USBD_RUNTIME_SERIAL_NUMBER)
#define UB_USBD_RUNTIME_SERIAL_NUMBER                           1
#endif

/** Whether to include support for high-speed USB connections */
#if !defined(UB_USBD_ENABLE_HIGH_SPEED)
#define UB_USBD_ENABLE_HIGH_SPEED                               0
#endif

/** Whether to enable consistency checks between static configuration and provided runtime implementations */
#if !defined(UB_USBD_ENABLE_TYPE_IDENTIFIERS)
#define UB_USBD_ENABLE_TYPE_IDENTIFIERS                         1
#endif

/** Whether PCD implementations should use interrupts */
#if !defined(UB_USBD_ENABLE_INTERRUPTS)
#define UB_USBD_ENABLE_INTERRUPTS                               1
#endif

// == Configuration checks and derived parameters: =====================================================================

#if (UB_USBD_MAX_IN_ENDPOINTS < 1) || (UB_USBD_MAX_OUT_ENDPOINTS < 1)
#error "UB_USBD_MAX_IN_ENDPOINTS and UB_USBD_MAX_OUT_ENDPOINTS must be at least 1 to support control endpoint"
#endif

#if UB_USBD_MAX_CONTROL_PACKET != 8 && UB_USBD_MAX_CONTROL_PACKET != 16 && UB_USBD_MAX_CONTROL_PACKET != 32 && \
    UB_USBD_MAX_CONTROL_PACKET != 64
#error "UB_USBD_MAX_CONTROL_PACKET must be either 8, 16, 32 or 64 per USB specification"
#endif

#if UB_USBD_MAX_FUNCTIONS < 1
#error "UB_USBD_MAX_FUNCTIONS must be at least 1"
#endif

#if UB_USBD_MAX_FUNCTIONS > 15
#error "UB_USBD_MAX_FUNCTIONS must be at most 15 due to implementation limit"
#endif

#if UB_USBD_MAX_INTERFACES < 1
#error "UB_USBD_MAX_INTERFACES must be at least 1"
#endif

#if UB_USBD_MAX_FUNC_ENDPOINTS < 1
#error "UB_USBD_MAX_FUNC_ENDPOINTS must be at least 1"
#endif

#if UB_USBD_MAX_FUNC_ENDPOINTS > 16
#error "UB_USBD_MAX_FUNC_ENDPOINTS must be at most 16"
#endif

/** Defines whether current configuration supports any additional data endpoints */
#if (UB_USBD_MAX_IN_ENDPOINTS == 1) && (UB_USBD_MAX_OUT_ENDPOINTS == 1)
#define UB_USBD_HAVE_DATA_ENDPOINTS                             0
#else
#define UB_USBD_HAVE_DATA_ENDPOINTS                             1
#endif

#if UB_USBD_HAVE_DATA_ENDPOINTS || (UB_USBD_MAX_FUNCTIONS > 1)
#define UB_USBD_HAVE_RESOURCE_MAPPING                           1
#else
#define UB_USBD_HAVE_RESOURCE_MAPPING                           0
#endif

#endif // UB_USB_DEVICE_BASE_USBD_CONFIG_H
