#include "pcd_fs_devices.hpp"

#if defined(UB_USBD_FS_CORE)

#include "pcd_fs.hpp"

using namespace ub::stm32::usb;

#if UB_USBD_FS_SRAM_ACCESS == 16
/**
 * std::memcpy variation which operates on half-words and bytes only.
 *
 * STM32G4 reference manual explicitly forbids 32-bit accesses to the USB packet memory (data is silently corrupted).
 */
static void usbd_memcpy(void *dst, const void *src, size_t length) {
    auto dst16 = (uint16_t *) dst;
    auto src16 = (const uint16_t *) src;
    auto end16 = src16 + (length >> 1);

    while (src16 != end16) {
        *dst16 = *src16;
        src16++;
        dst16++;
    }

    if ((length & 1) != 0) {
        *((uint8_t *) dst16) = *((const uint8_t *) src16);
    }
}

void impl::usbd_readPacket(void *dst, const void *src, size_t length) {
    usbd_memcpy(dst, src, length);
}

void impl::usbd_writePacket(void *dst, const void *src, size_t length) {
    usbd_memcpy(dst, src, length);
}

#endif // UB_USBD_FS_SRAM_ACCESS == 16

#if UB_USBD_FS_SRAM_ACCESS == 32

/**
 * std::memcpy variation which reads 32-bit words only, and specifically handles incomplete trailing word.
 * USB stack has no transmit/receive buffer alignment requirements, so it is an error to overwrite destination buffer
 * past its declared length.
 *
 * STM32H5 reference manual explicitly forbids any accesses other than 32-bit to the USB packet memory (data is
 * silently corrupted).
 */
void impl::usbd_readPacket(void *dst, const void *src, size_t length) {
    auto dst32 = (uint32_t *) dst;
    auto src32 = (const uint32_t *) src;
    auto end32 = src32 + (length >> 2);

    while (src32 != end32) {
        *dst32 = *src32;
        src32++;
        dst32++;
    }

    uint32_t tail = length & 3;
    if (tail != 0) {
        auto dst8 = (uint8_t *) dst32;
        auto end8 = dst8 + tail;

        uint32_t srcW = *src32;
        while (dst8 != end8) {
            *dst8 = srcW;
            srcW = srcW >> 8;
            dst8++;
        }
    }
}

/**
 * std::memcpy variation which writes 32-bit words only.
 *
 * STM32H5 reference manual explicitly forbids any accesses other than 32-bit to the USB packet memory (data is
 * silently corrupted).
 */
void impl::usbd_writePacket(void *dst, const void *src, size_t length) {
    auto dst32 = (uint32_t *) dst;
    auto src32 = (const uint32_t *) src;
    auto end32 = src32 + (length >> 2);

    while (src32 != end32) {
        *dst32 = *src32;
        src32++;
        dst32++;
    }

    uint32_t tail = length & 3;
    if (tail != 0) {
        // This should not cause any hardware fault. Packet length is explicitly defined, so peripheral
        // won't send any excess data.
        *dst32 = *src32;
    }
}

#endif // UB_USBD_FS_SRAM_ACCESS == 32

#endif
