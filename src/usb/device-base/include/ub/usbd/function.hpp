#ifndef UB_USB_DEVICE_BASE_USBD_FUNCTION_H
#define UB_USB_DEVICE_BASE_USBD_FUNCTION_H

#include <ub/usbd/config.hpp>
#include <ub/usbd/model.hpp>
#include <ub/usbd/control.hpp>

namespace ub::usbd {
    /**
     * Interface which USB stack provides for functions to interact with interface.
     *
     * Function implementations use logical numbers for endpoints and interfaces.
     */
    struct FunctionHost {
        /** @return Speed of currently established link */
        [[nodiscard]] virtual LinkSpeed linkSpeed() const = 0;

        /**
         * Set or clear endpoint's STALL status.
         *
         * @param endpoint Logical endpoint index
         * @param stall    Desired STALL status
         */
        virtual void stallEndpoint(uint8_t endpoint, bool stall) = 0;

        /**
         * @return Whether endpoint is currently stalled.
         */
        virtual bool stalled(uint8_t endpoint) = 0;

        /**
         * Prepare endpoint to receive single packet. Packet will be placed in specified buffer on reception. Length of
         * buffer is implicitly assumed to be the maximum packet length configured for the endpoint. Buffer must remain
         * valid until either `shutdown()` or `packetReceived()` is called on the function.
         *
         * Function must never call this method twice on same endpoint without first receiving
         * `packetReceived()` callback.
         *
         * @param endpoint Logical endpoint index
         * @param buffer   Buffer to place received packet
         */
        virtual void receivePacket(uint8_t endpoint, void *buffer) = 0;

        /**
         * Schedule packet to be transmitted from the endpoint. Buffer must remain valid until either `shutdown()` or
         * `transmitComplete` is called on the function object. Packets larger than maximum configured packet length
         * are silently truncated.
         *
         * Functions must never call `transmitPacket` twice on same endpoint without receiving `transmitComplete`
         * callback.
         *
         * @param endpoint Logical endpoint index
         * @param buffer   Buffer with data to transmit
         * @param length   Length to transmit.
         */
        virtual void transmitPacket(uint8_t endpoint, const void *buffer, uint32_t length) = 0;
    };

    /**
     * Helper class to improve encapsulation of the internal USB stack callbacks from external user code.
     *
     * Class could implement both Function and FunctionLogic class with zero overhead by using private inheritance:
     *
     *   class Foo: public Function, private FunctionLogic { ... }
     *
     * This way, all internal USB methods would stay hidden from the user, but will still be accessible to USB stack
     * internals.
     */
    struct FunctionLogic: public ControlHandler {
        /**
         * Callback indicating that packet has been received on the endpoint.
         */
        virtual void packetReceived(uint8_t endpoint, size_t length) = 0;

        /**
         * Callback indicating that packet has been successfully transmitted.
         */
        virtual void transmitComplete(uint8_t endpoint) = 0;
    };

    struct Function {
#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
        /**
         * Return a hardcoded function type identifier.
         *
         * This identifier is used to verify consistency between runtime-registered functions and functions compiled in
         * the descriptor.
         *
         * Descriptor compiler derives function type in the following way:
         *  1. Take human-readable function type name, i.e. 'serial-port.v1'
         *  2. Encode this name as UTF-8 and compute SHA-256 hash of it
         *  3. Interpret first 4 bytes of the hash as a big-endian integer (0xDD1BEB43 for serial port example)
         */
        [[nodiscard]] virtual uint32_t functionType() const = 0;
#endif

        /**
         * Reset function state and create function logic instance.
         *
         * This method is called after host configures device on each USB reset.
         */
        virtual FunctionLogic *initialize(FunctionHost &host, THROWS) = 0;
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_FUNCTION_H
