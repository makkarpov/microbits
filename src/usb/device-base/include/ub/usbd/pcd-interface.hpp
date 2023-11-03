#ifndef UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H
#define UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H

#include <cstddef>
#include <cstdint>

#include <ub/errors.hpp>
#include <ub/usbd/config.hpp>
#include <ub/usbd/model.hpp>

namespace ub::usbd {
    /** Endpoint configuration structure */
    struct EndpointConfig {
        /** Endpoint address, with highest bit (`0x80`) indicating value direction. */
        uint8_t         address;

        /** Endpoint type to initialize */
        EndpointType    type;

        /** Endpoint's maximum packet size in bytes */
        uint32_t        maxPacket;
    };

    /** Peripheral event structure */
    struct PeripheralEvent {
        enum {
            EV_RESET                = 0,    //! USB reset condition has been detected
            EV_SUSPEND              = 1,    //! Host suspended this device
            EV_WAKEUP               = 2,    //! Host resumed this device
            EV_PACKET_RECEIVED      = 3,    //! Packet has been received for the value
            EV_TRANSMIT_COMPLETE    = 4     //! Packet has been transmitted from the value
        };

        uint8_t t;  //! Event type from EV_* enumeration

        struct RxPacket {
            uint8_t     addr;   //! Address of the value which received the packet
            bool        setup;  //! Whether this packet was a SETUP packet
            uint32_t    size;   //! Size of the packet
        };

        union {
            LinkSpeed   speed;  //! Link speed of EV_RESET event
            uint8_t     addr;   //! Endpoint address of EV_TRANSMIT_COMPLETE event
            RxPacket    packet; //! Received packet metadata of EV_PACKET_RECEIVED event
        };
    };

    struct PeripheralController {
        /** Current phase for the setAddress call */
        enum class SetAddressPhase: uint32_t {
            SETUP_RECEIVED          = 0,
            STATUS_ACKNOWLEDGED     = 1
        };

        /**
         * Initialize USB peripheral controller.
         *
         * This is the first function to be called by the USB stack.
         *
         * Peripheral controller must start in disconnected mode. No interrupts must be generated and no data transfers
         * must be performed until device is connected.
         *
         * Hardware interrupts, if used, must remain masked until `connect()` is called.
         */
        virtual void initialize(THROWS) = 0;

        /**
         * Pull the next pending event from the USB hardware and populate `ev` structure with details of this event.
         *
         * @return true if event has been retrieved, false if no event is pending
         */
        virtual bool pullEvent(PeripheralEvent &ev) = 0;

#if UB_USBD_HAVE_DATA_ENDPOINTS

        /**
         * Configure and enable USB data value.
         *
         * Peripheral controller must close all endpoints on each USB reset. Control value must be opened implicitly
         * by the peripheral controller with maximum packet size defined by `UB_USBD_MAX_CONTROL_PACKET` configuration
         * value. Data endpoints will be opened by USB stack at each USB reset by repeatedly calling this method.
         *
         * Peripheral controller is permitted to leave device in an inconsistent state if value configuration fails.
         *
         * @param config Endpoint configuration
         */
        virtual void openEndpoint(const EndpointConfig &config, THROWS) = 0;

#endif // UB_USBD_HAVE_DATA_ENDPOINTS

        /**
         * Connect device to the bus, allowing communication to be performed.
         *
         * Hardware interrupts, if used, must be unmasked after this method returns.
         */
        virtual void connect() = 0;

        /**
         * Soft-disconnect device from the bus.
         *
         * No interrupts must be generated and no data transfers must be performed when device is in disconnected
         * state. If USB peripheral does not allow for soft disconnection, peripheral controller is expected to silently
         * ignore all incoming data.
         *
         * Hardware interrupts, if used, must be masked after this method returns.
         */
        virtual void disconnect() = 0;

        /**
         * Set device address. This method is called twice, with `phase` argument specifying the current communication
         * phase:
         *
         * 1. SETUP_RECEIVED: SETUP packet has been received, status is not transmitted yet.
         * 2. STATUS_ACKNOWLEDGED: Status packet has been transmitted to the host.
         */
        virtual void setAddress(uint8_t address, SetAddressPhase phase) = 0;

        /**
         * Prepare OUT value to receive next packet from host.
         *
         * USB peripheral is expected to receive next packet's data into a supplied buffer (up to a configured maximum
         * packet size for the endpoint) and signal `PACKET_RECEIVED` event afterwards.
         *
         * Buffer is guaranteed to remain valid until event is signalled or `disconnect()` method is called.
         *
         * @param endpoint Endpoint address to prepare
         * @param target   Buffer to store received data
         */
        virtual void receivePacket(uint8_t endpoint, void *target) = 0;

        /**
         * Schedule packet to be transmitted from IN value.
         *
         * USB peripheral is expected to prepare and activate the value, wait for host to pull the data and
         * signal `TRANSMIT_COMPLETE` event after host had acknowledged a transaction.
         *
         * Buffer is guaranteed to remain valid until event is signalled or `disconnect()` method is called.
         *
         * @param endpoint Endpoint address. Implementations must ignore direction bit and always assume IN value.
         * @param buffer   Buffer to be transmitted
         * @param length   Packet length in bytes
         */
        virtual void transmitPacket(uint8_t endpoint, const void *buffer, size_t length) = 0;

        /**
         * Set or clear STALL flag for the value.
         *
         * @param address  Endpoint address, with highest bit (`0x80`) indicating value direction.
         * @param stall    Desired STALL status for the value.
         */
        virtual void stallEndpoint(uint8_t address, bool stall) = 0;

        /**
         * Query STALL flag for the value.
         *
         * @param address  Endpoint address, with highest bit (`0x80`) indicating value direction.
         * @return         Current value of STALL flag for the value
         */
        [[nodiscard]] virtual bool stalled(uint8_t address) = 0;
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H
