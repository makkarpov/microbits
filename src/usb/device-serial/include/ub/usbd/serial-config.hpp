#ifndef UB_USB_DEVICE_SERIAL_USBD_SERIAL_CONFIG_H
#define UB_USB_DEVICE_SERIAL_USBD_SERIAL_CONFIG_H

#include <ub/user-config.h>

/** Maximum packet length for CDC endpoints */
#if !defined(UB_USBD_SERIAL_PACKET_LENGTH)
#define UB_USBD_SERIAL_PACKET_LENGTH                            64
#endif

#endif // UB_USB_DEVICE_SERIAL_USBD_SERIAL_CONFIG_H
