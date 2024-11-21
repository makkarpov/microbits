#ifndef UB_SRC_STM32_USB_DEVICE_PCD_FS_H
#define UB_SRC_STM32_USB_DEVICE_PCD_FS_H

#include <ub/usb-device.hpp>

#include "pcd_fs_devices.hpp"

#define USB_R_CNTR_USBRST               (1 << 0)
#define USB_R_CNTR_RESETM               (1 << 10)
#define USB_R_CNTR_CTRM                 (1 << 15)

#define USB_R_ISTR_EP_ID                15
#define USB_R_ISTR_DIR                  (1 << 4)
#define USB_R_ISTR_RESET                (1 << 10)
#define USB_R_ISTR_CTR                  (1 << 15)

#define USB_R_DADDR_EF                  (1 << 7)
#define USB_R_BCDR_DPPU                 (1 << 15)

#define USB_R_EPnR_ADDR                 15
#define USB_R_EPnR_STAT_Msk             3
#define USB_R_EPnR_STAT_STALL_Val       1
#define USB_R_EPnR_STAT_NAK_Val         2
#define USB_R_EPnR_STAT_VALID_Val       3
#define USB_R_EPnR_STAT_TX_Pos          4
#define USB_R_EPnR_STAT_TX              (USB_R_EPnR_STAT_Msk << USB_R_EPnR_STAT_TX_Pos)
#define USB_R_EPnR_DTOG_TX              (1 << 6)
#define USB_R_EPnR_CTR_TX               (1 << 7)
#define USB_R_EPnR_EP_KIND              (1 << 8)
#define USB_R_EPnR_TYPE_Pos             9
#define USB_R_EPnR_TYPE_Msk             3
#define USB_R_EPnR_TYPE                 (USB_R_EPnR_TYPE_Msk << USB_R_EPnR_TYPE_Pos)
#define USB_R_EPnR_TYPE_BULK_Val        0
#define USB_R_EPnR_TYPE_CONTROL_Val     1
#define USB_R_EPnR_TYPE_ISOCHRONOUS_Val 2
#define USB_R_EPnR_TYPE_INTERRUPT_Val   3
#define USB_R_EPnR_SETUP                (1 << 11)
#define USB_R_EPnR_STAT_RX_Pos          12
#define USB_R_EPnR_STAT_RX              (USB_R_EPnR_STAT_Msk << USB_R_EPnR_STAT_RX_Pos)
#define USB_R_EPnR_DTOG_RX              (1 << 14)
#define USB_R_EPnR_CTR_RX               (1 << 15)

// =====================================================================================================================

// A single check to avoid having #ifdef's all over the place
#if !defined(UB_USBD_FS_CORE)
#error "UB_USBD_FS_CORE is not defined -- wrap inclusion of this file into #ifdef"
#endif

#if UB_USBD_FS_SRAM_ACCESS == 16
struct USB_BufferDescriptor {
    volatile uint16_t ADDR_TX;
    volatile uint16_t COUNT_TX;
    volatile uint16_t ADDR_RX;
    volatile uint16_t COUNT_RX;
};

constexpr static inline size_t usbd_alignLength(size_t address) {
    return (address + 1) & -2;
}

static inline void usbd_setTxDescriptor(USB_BufferDescriptor &desc, size_t addr, size_t length) {
    desc.ADDR_TX = addr;
    desc.COUNT_TX = length;
}

static inline void usbd_setRxDescriptor(USB_BufferDescriptor &desc, size_t addr, uint16_t size) {
    desc.ADDR_RX = addr;
    desc.COUNT_RX = count << 10;
}

static inline size_t usbd_readRxLength(USB_BufferDescriptor &desc) {
    return desc.COUNT_RX & 0x3FF;
}
#elif UB_USBD_FS_SRAM_ACCESS == 32
struct USB_BufferDescriptor {
    volatile uint32_t DESC_TX;
    volatile uint32_t DESC_RX;
};

constexpr static inline size_t usbd_alignLength(size_t address) {
    return (address + 3) & -4;
}

static inline void usbd_setTxDescriptor(USB_BufferDescriptor &desc, size_t addr, size_t length) {
    desc.DESC_TX = (length << 16) | addr;
}

static inline void usbd_setRxDescriptor(USB_BufferDescriptor &desc, size_t addr, uint16_t size) {
    desc.DESC_RX = ((uint32_t) size << 26) | addr;
}

static inline size_t usbd_readRxLength(USB_BufferDescriptor &desc) {
    return (desc.DESC_RX >> 16) & 0x3FF;
}
#else
#error "UB_USBD_FS_SRAM_ACCESS is not defined or is invalid"
#endif

static_assert(sizeof(USB_BufferDescriptor) == 8);

// =====================================================================================================================

#define USB_N_ENDPOINTS                 8
#define USB_R_EPnR                      ((volatile uint32_t *) &(USB_R_EP0_BASE))
#define USB_BUFFER_TABLE                ((USB_BufferDescriptor *) USB_MEMORY)
#define USB_BUFFER_TABLE_LEN            (sizeof(USB_BufferDescriptor) * (USB_N_ENDPOINTS))

namespace ub::stm32::usb::impl {
    /** Copy packet from the dedicated packet SRAM using access methods allowed by the current hardware */
    void usbd_readPacket(void *dst, const void *src, size_t length);

    /** Copy packet into the dedicated packet SRAM using access methods allowed by the current hardware */
    void usbd_writePacket(void *dst, const void *src, size_t length);
}

#endif // UB_SRC_STM32_USB_DEVICE_PCD_FS_H
