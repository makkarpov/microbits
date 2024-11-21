#ifndef UB_STM32_USB_DEVICE_STM32_USB_DEVICE_H
#define UB_STM32_USB_DEVICE_STM32_USB_DEVICE_H

#include <ub/stm32.h>

#include <ub/usbd/config.hpp>
#include <ub/usbd/pcd-interface.hpp>

#define UB_STM32_USBD_USB_DEVICE_INCLUDED

#if defined(STM32G4) || defined(STM32H5)
#include <ub/stm32/usbd/usb-device-fs.hpp>
#else
#error "Unsupported STM32 device"
#endif

#endif // UB_STM32_USB_DEVICE_STM32_USB_DEVICE_H
