#ifndef UB_SRC_STM32_USB_DEVICE_PCD_STM32G4_H
#define UB_SRC_STM32_USB_DEVICE_PCD_STM32G4_H

#include <cstdint>

struct USB_BufferDescriptor {
    volatile uint16_t ADDR_TX;
    volatile uint16_t COUNT_TX;
    volatile uint16_t ADDR_RX;
    volatile uint16_t COUNT_RX;
};

static_assert(sizeof(USB_BufferDescriptor) == 8);

#define USB_EPnR(n)             *((volatile uint16_t *) &(USB->EP0R) + 2 * (n))
#define USB_MEMORY              ((uint8_t *) 0x40006000)
#define USB_MEMORY_LENGTH       1024

#define USB_BUFFER_TABLE        ((volatile USB_BufferDescriptor *) USB_MEMORY)
#define USB_BUFFER_TABLE_LEN    64      // 8 endpoints * 4 half-words each

#define USB_N_ENDPOINTS         8

#endif // UB_SRC_STM32_USB_DEVICE_PCD_STM32G4_H
