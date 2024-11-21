#ifndef UB_SRC_STM32_USB_DEVICE_PCD_FS_DEVICES_H
#define UB_SRC_STM32_USB_DEVICE_PCD_FS_DEVICES_H

#include <ub/stm32.h>
#include <ub/user-config.h>

#if defined(STM32G4)
#define UB_USBD_FS_CORE
#define UB_USBD_FS_SRAM_ACCESS          16

#define USB_MEMORY                      ((uint8_t *) 0x40006000)
#define USB_MEMORY_SIZE                 1024

#define USB_R_CNTR                      USB->CNTR
#define USB_R_ISTR                      USB->ISTR
#define USB_R_DADDR                     USB->DADDR
#define USB_R_BCDR                      USB->BCDR
#define USB_R_BTABLE                    USB->BTABLE
#define USB_R_EP0_BASE                  USB->EP0R
#endif

#if defined(STM32H5)
#define UB_USBD_FS_CORE
#define UB_USBD_FS_SRAM_ACCESS          32

#define USB_MEMORY                      ((uint8_t *) 0x40016400)
#define USB_MEMORY_LENGTH               2048

#define USB_R_CNTR                      USB_DRD_FS->CNTR
#define USB_R_ISTR                      USB_DRD_FS->ISTR
#define USB_R_DADDR                     USB_DRD_FS->DADDR
#define USB_R_BCDR                      USB_DRD_FS->BCDR
#define USB_R_EP0_BASE                  USB_DRD_FS->CHEP0R

// STM32H5 errata requires a minimum 800ns delay between reception of CTR interrupt and reading of the packet SRAM,
// otherwise corrupted data might be read.
#if !defined(UB_STM32_USB_H5_ERRATA_WORKAROUND)
#define UB_STM32_USB_H5_ERRATA_WORKAROUND   1
#endif

#endif

#endif // UB_SRC_STM32_USB_DEVICE_PCD_FS_DEVICES_H
