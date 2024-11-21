#ifndef UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H
#define UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H

#include <cstddef>
#include <cstdint>

#include <ub/errors.hpp>
#include <ub/usbd/config.hpp>
#include <ub/usbd/model.hpp>

namespace ub::usbd {
    /** Peripheral event structure */
    struct PeripheralEvent {
        enum {
            /**
             * USB reset condition has been detected on the bus.
             *
             * Peripheral controller must populate `speed` field to indicate actual enumerated bus speed. Control
             * endpoint must be already active when event is fired. `speed` must not be `LinkSpeed::NONE`.
             */
            EV_RESET                = 0,

            /**
             * USB SUSPEND condition (lack of activity for longer than 3ms) has been detected on the bus. This could
             * also indicate disconnection, which is indistinguishable from suspend without Vbus sensing.
             */
            EV_SUSPEND              = 1,

            /** USB activity resumed after being suspended */
            EV_WAKEUP               = 2,

            /**
             * Packet has been received on some endpoint. Peripheral controller must populate `packet` field to
             * further specify endpoint index, SETUP or OUT transaction type and packet length in bytes.
             *
             * USB stack assumes that PCD only triggers this event for endpoints which have been explicitly opened
             * and for which `receivePacket()` call has been made by the USB stack. Control endpoint is assumed to be
             * always open. Each `receivePacket()` call must eventually result in exactly one `EV_PACKET_RECEIVED`
             * event. Each USB reset implicitly closes all data endpoints and aborts any pending receptions.
             * No events should be issued for receptions interrupted by USB reset.
             */
            EV_PACKET_RECEIVED      = 3,

            /**
             * Packet has been successfully transmitted and acknowledged by the host.
             *
             * Peripheral controller must populate `endpoint` field to indicate endpoint index without direction bit.
             *
             * USB stack assumes that PCD only triggers this event for endpoints which have been explicitly opened
             * and for which `transmitPacket()` call has been made by the USB stack. Control endpoint is assumed to be
             * always open. Each `transmitPacket()` call must eventually result in exactly one `EV_TRANSMIT_COMPLETE`
             * event. Each USB reset implicitly closes all data endpoints and aborts any outstanding transmissions.
             * No events should be issued for transmissions interrupted by USB reset.
             */
            EV_TRANSMIT_COMPLETE    = 4
        };

        uint8_t t;  //! Event type from EV_* enumeration

        struct RxPacket {
            uint8_t     addr;   //! Address of the endpoint which received the packet
            bool        setup;  //! Whether this packet was a SETUP packet
            uint32_t    size;   //! Size of the packet
        };

        union {
            LinkSpeed   speed;  //! Link speed of EV_RESET event
            uint8_t     addr;   //! Endpoint address of EV_TRANSMIT_COMPLETE event
            RxPacket    packet; //! Received packet metadata of EV_PACKET_RECEIVED event
        };
    };

    /**
     * Base interface for peripheral controller drivers (PCDs), which are responsible for physical layer of the USB
     * communication.
     */
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
         * Peripheral controller is responsible for initialization of the control endpoint. By the time device connects
         * itself to the bus, control endpoint must be already configured and ready to accept transactions. Configured
         * value for UB_USBD_MAX_CONTROL_PACKET parameter must be used to set packet size for the control endpoint.
         * Control endpoint must respond with NAKs until `receivePacket()` method is called on it.
         *
         * Hardware interrupts, if used, must remain masked until `connect()` is called. Device must not signal any
         * events via pullEvent() function until it is connected.
         */
        virtual void initialize(THROWS) = 0;

        /**
         * Pull the next pending event from the USB hardware and populate `ev` structure with details of this event.
         *
         * @return true if event has been retrieved, false if no event is pending
         */
        virtual bool pullEvent(PeripheralEvent &ev) = 0;

        /**
         * Transition the device into a configured state, initialize and open all required data endpoints. Opened data
         * endpoints must be implicitly closed by the controller on next USB reset, and controller must be ready to
         * open same endpoints again.
         *
         * Exact specification of the endpoints is provided via `configData` parameter, which points to an
         * implementation defined data structure (such as ub::usbd::config::GenericTargetData). Actual content of that
         * structure is generated by the descriptor compiler.
         *
         * Peripheral controller is permitted to leave device in an inconsistent state if endpoint configuration fails.
         * If this happens, no events related to these data endpoints must be reported by the pullEvent() method.
         *
         * @param configData Value of the StaticConfiguration.targetData field
         * @param speedIndex Index of the current negotiated link speed
         */
        virtual void configureDevice(const void *configData, uint8_t speedIndex, THROWS) = 0;

        /**
         * Connect device to the bus, allowing communication to be performed.
         *
         * Hardware interrupts, if used, must be unmasked after this method returns.
         */
        virtual void connect() = 0;

        /**
         * Soft-disconnect device from the bus, aborting all pending transfers.
         *
         * No events must be generated and no data transfers must be performed when device is in disconnected state.
         * If USB peripheral does not allow for soft disconnection, peripheral controller is expected to silently
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
         * Prepare OUT endpoint to receive next packet from host.
         *
         * USB peripheral is expected to receive next packet's data into a supplied buffer (up to a configured maximum
         * packet size for the endpoint) and signal `PACKET_RECEIVED` event afterwards.
         *
         * Buffer is guaranteed to remain valid until event is signalled or `disconnect()` method is called.
         *
         * Peripheral controller can assume that USB stack never calls this method twice for same endpoint without
         * first receiving `EV_PACKET_RECEIVED` event.
         *
         * @param endpoint Endpoint address to prepare
         * @param target   Buffer to store received data
         */
        virtual void receivePacket(uint8_t endpoint, void *target) = 0;

        /**
         * Schedule packet to be transmitted from IN endpoint.
         *
         * USB peripheral is expected to prepare and activate the endpoint, wait for host to pull the data and
         * signal `TRANSMIT_COMPLETE` event after host had acknowledged a transaction.
         *
         * Buffer is guaranteed to remain valid until event is signalled or `disconnect()` method is called.
         *
         * Peripheral controller can assume that USB stack never calls this method twice for same endpoint without
         * first receiving `EV_TRANSMIT_COMPLETE` event.
         *
         * @param endpoint Endpoint address. Implementations must ignore direction bit and always assume IN endpoint.
         * @param buffer   Buffer to be transmitted
         * @param length   Packet length in bytes
         */
        virtual void transmitPacket(uint8_t endpoint, const void *buffer, size_t length) = 0;

        /**
         * Set or clear STALL flag for the endpoint.
         *
         * @param address  Endpoint address, with highest bit (`0x80`) indicating endpoint direction.
         * @param stall    Desired STALL status for the endpoint.
         */
        virtual void stallEndpoint(uint8_t address, bool stall) = 0;

        /**
         * Query STALL flag for the endpoint.
         *
         * @param address  Endpoint address, with highest bit (`0x80`) indicating endpoint direction.
         * @return         Current value of STALL flag for the endpoint
         */
        [[nodiscard]] virtual bool stalled(uint8_t address) = 0;
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_PCD_INTERFACE_H
