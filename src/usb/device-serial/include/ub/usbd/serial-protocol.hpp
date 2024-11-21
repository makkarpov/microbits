#ifndef UB_USB_DEVICE_SERIAL_USBD_SERIAL_PROTOCOL_H
#define UB_USB_DEVICE_SERIAL_USBD_SERIAL_PROTOCOL_H

#include <ub/compiler.hpp>

#include <cstdint>
#include <cstddef>

namespace ub::usbd::serial {
    PACKED_STRUCT LineCoding {
        constexpr static uint32_t LENGTH = 7;

        constexpr explicit LineCoding(): dwLineRate(0), bCharFormat(0), bParityType(0), bDataBits(0) {}

        constexpr static uint8_t CHAR_STOP_1        = 0x00; //! bCharFormat value for 1 stop bit
        constexpr static uint8_t CHAR_STOP_1_5      = 0x01; //! bCharFormat value for 1.5 stop bits
        constexpr static uint8_t CHAR_STOP_2        = 0x02; //! bCharFormat value for 2 stop bits

        constexpr static uint8_t PARITY_NONE        = 0x00; //! bParityType value for none parity
        constexpr static uint8_t PARITY_ODD         = 0x01; //! bParityType value for odd parity
        constexpr static uint8_t PARITY_EVEN        = 0x02; //! bParityType value for even parity
        constexpr static uint8_t PARITY_MARK        = 0x03; //! bParityType value for mark parity
        constexpr static uint8_t PARITY_SPACE       = 0x04; //! bParityType value for space parity

        uint32_t dwLineRate;        //! Data terminal rate, in bits per second
        uint8_t  bCharFormat;       //! Char format (stop bits)
        uint8_t  bParityType;       //! Parity type
        uint8_t  bDataBits;         //! Data bits
    };
}

#endif // UB_USB_DEVICE_SERIAL_USBD_SERIAL_PROTOCOL_H
