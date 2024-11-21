#ifndef UB_USB_DEVICE_BASE_USBD_MODEL_H
#define UB_USB_DEVICE_BASE_USBD_MODEL_H

#include <cstdint>
#include <cstddef>

namespace ub::usbd {
    /** IN (device -> host) endpoint direction flag */
    constexpr static uint8_t EP_IN  = 0x80;

    /** OUT (host -> device) endpoint direction flag */
    constexpr static uint8_t EP_OUT = 0x00;

    /** Address of control IN endpoint */
    constexpr static uint8_t EP_CONTROL_IN = 0x00 | EP_IN;

    /** Address of control OUT endpoint */
    constexpr static uint8_t EP_CONTROL_OUT = 0x00 | EP_OUT;

    /** Endpoint number mask */
    constexpr static uint8_t EP_NUM = 0x0F;

    /** Endpoint type enumeration. Numeric values are consistent with values used in endpoint descriptors. */
    enum class EndpointType: uint8_t {
        // Control endpoints are opened by the stack
        ISOCHRONOUS = 1,
        BULK        = 2,
        INTERRUPT   = 3
    };

    /** USB link speed */
    enum class LinkSpeed: uint8_t {
        NONE        = 0,            //! No link has been established yet
        LOW         = 1,            //! USB 2.0 low speed (1.5 Mbps)
        FULL        = 2,            //! USB 2.0 full speed (12 Mbps)
        HIGH        = 3             //! USB 2.0 high speed (480 Mbps)
    };

    /** Setup request type */
    enum class SetupType: uint8_t {
        STANDARD    = 0,    //! Setup request defined by USB standard
        CLASS       = 1,    //! Setup request defined by implemented USB class
        VENDOR      = 2     //! Setup request defined by device vendor
    };

    /** Setup request recipient */
    enum class SetupRecipient: uint8_t {
        DEVICE      = 0,    //! Setup request is directed to whole device
        INTERFACE   = 1,    //! Setup request is directed to interface
        ENDPOINT    = 2     //! Setup request is directed to endpoint
    };

    /** Setup packet structure as it is serialized on wire */
    struct SetupPacket {
        uint8_t  bmRequestType;     //! Packed characteristics of request
        uint8_t  bRequest;          //! Request method number
        uint16_t wValue;            //! Interpretation depends on request
        uint16_t wIndex;            //! Interpretation depends on request
        uint16_t wLength;           //! Request length or maximum expected response length

        constexpr static uint32_t LENGTH = 8;   //! Setup packet length on wire

        /** @return Whether this device is going to transmit data stage */
        [[nodiscard]] inline bool deviceToHost() const { return (bmRequestType & 0x80) != 0; }

        /** @return Type of setup request */
        [[nodiscard]] inline SetupType type() const { return (SetupType) ((bmRequestType >> 5) & 0x3); }

        /** @return Recipient of setup request */
        [[nodiscard]] inline SetupRecipient recipient() const { return (SetupRecipient) (bmRequestType & 0xF); }
    };

    static_assert(sizeof(SetupPacket) == SetupPacket::LENGTH);
}

#endif // UB_USB_DEVICE_BASE_USBD_MODEL_H
