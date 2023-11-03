#ifndef UB_USB_DEVICE_BASE_USBD_CONTROL_H
#define UB_USB_DEVICE_BASE_USBD_CONTROL_H

#include <ub/usbd/errors.hpp>
#include <ub/usbd/model.hpp>

namespace ub::usbd {
    struct ControlStreamer;

    /**
     * Control request handler and state machine.
     *
     * This class acts both as public interface for application-level control handlers and as actual implementation.
     * This is done to reduce overhead for virtual method table where only one implementation exists
     */
    class ControlEndpoint {
    public:
        // Public interface for control handlers:

        /** @return Currently handled setup packet */
        [[nodiscard]] virtual const SetupPacket &setupPacket() const = 0;

        /** @return Packet buffer (shared for incoming and outgoing data) */
        [[nodiscard]] virtual uint8_t *packetBuffer() = 0;

        /** @return Packet length */
        [[nodiscard]] virtual size_t packetLength() const = 0;

        /** @return Current USB link speed */
        [[nodiscard]] virtual LinkSpeed linkSpeed() const = 0;

        /**
         * Receive next data packet into the packet buffer. `packetReceived()` callback will be called on
         * packet arrival. Should never be called on outbound requests.
         */
        virtual void receivePacket() = 0;

        /**
         * Transmit next data packet. Buffer must remain valid until `transmitComplete()` or `aborted()` callback is
         * called. Transmitted length must not exceed maximum packet length. Should never be called on inbound requests.
         *
         * Note that per USB specification, end of transfer is signalled with less-than-maximum-length packet.
         * Therefore, only last packet could have length less than value maximum. All preceding packets must have
         * full length. Control state machine will use this logic to detect a moment to initiate status phase.
         *
         * It may be necessary to transmit zero length packet if response payload length is a multiple of packet length.
         *
         * @param buffer Buffer to transmit
         * @param length Packet length
         */
        virtual void transmitPacket(const void *buffer, size_t length) = 0;

        /**
         * Abort request processing and send an error response to host.
         */
        virtual void abortRequest() = 0;
    };

    /** Expected data phase direction */
    enum class ControlDirection {
        INBOUND     = 0,                    //! Handler expects to receive data
        OUTBOUND    = 1                     //! Handler expects to send data
    };

    /** Control request preparation context */
    struct ControlRequest {
        const SetupPacket   *setup;         //! Received setup packet
        bool                accepted;       //! Handler should set this to 'true' to accept the setup request
        ControlDirection    direction;      //! Expected data transfer direction
        uint32_t            maxLength;      //! Maximum data length application is willing to accept
        ControlEndpoint     *endpoint;      //! Control value to use while streaming
        ControlStreamer     *streamer;      //! If advanced processing is desired, streamer pointer must be placed here

        /** Reset all optional fields to their default value */
        void reset();
    };

    /** Control request handler interface */
    struct ControlHandler {
        /**
         * Decode, interpret and prepare to handle control request.
         *
         * If request is unknown or malformed, this method should simply return without doing anything. Otherwise, it
         * should set `request.accepted` flag to indicate that processing should be continued. When accepting the
         * request, handler should populate `direction` and `maxLength` fields with values suitable for the expected
         * data phase. USB stack will enforce these parameters on actual data stage and reject the request upon
         * detecting a mismatch.
         *
         * Upon accepting, application has two ways to continue with request processing:
         *
         *   1. When request is a simple request which does not require data streaming, this method should leave
         *      `request.streamer` field in the default state (`nullptr`). Request data must fit into a single packet,
         *      otherwise request will be rejected. When request is ready to be handled (i.e. data phase is received),
         *      `handleControl` method will be invoked to do actual processing.
         *
         *   2. When request either requires data streaming or completion callback, `ControlStreamer` instance must be
         *      provided via `request.streamer` field. Streamer should use `request.value` instance to interact with
         *      device control value. Streamer instance must stay valid until `completed()` or `aborted()` is invoked
         *      on the streamer.
         */
        virtual void setupControl(ControlRequest &request) = 0;

        /**
         * Handle a simple control request. This method is guaranteed to be preceded by a relevant successful
         * `prepareControl` invocation. User could extract any parameters in `prepareControl` and reuse it here.
         *
         * When data transfer is inbound, `buffer` is filled with incoming packet data and `length` is set to actual
         * packet length. Otherwise, implementation must fill `buffer` with response and set `length` parameter.
         *
         * Any thrown error will cause request to be rejected without sending any data.
         *
         * @param setup  Setup packet
         * @param buffer Buffer with received data or a buffer to fill with transmitted data
         * @param length Received data length or a placeholder to set response length
         * @return       `true` if request is processed successfully
         */
        virtual void handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) = 0;

        /**
         * Complete control request.
         *
         * @param setup   Setup packet
         * @param success Whether the completion was successful
         */
        virtual void completeControl(const SetupPacket &setup, bool success) {};
    };

    /**
     * Advanced control request handler.
     *
     * All functions could be invoked in the interrupt context.
     */
    struct ControlStreamer {
        /**
         * Called when status phase had been sent (inbound requests) or received (outbound requests).
         *
         * This is the last method to be called, streamer instance could be safely destroyed afterwards.
         */
        virtual void completed() = 0;

        /**
         * Called when current request processing aborts for some reason.
         *
         * This is the last method to be called, streamer instance could be safely destroyed afterwards.
         */
        virtual void aborted() = 0;

        /**
         * Data packet has been received. Never called on outbound requests.
         *
         * @param buffer Packet buffer. This is the same buffer as `value.packetBuffer()` and provided as a parameter
         *               for implementation convenience
         * @param length Length of received packet
         */
        virtual void packetReceived(const uint8_t *buffer, uint32_t length) = 0;

        /**
         * Last transmitted packet has been sent successfully. Also invoked at the start to initiate the transmission.
         *
         * Never called on inbound requests.
         */
        virtual void transmitComplete() = 0;
    };

    /** Inbound control streamer with default method implementations */
    struct InboundControlStreamer: public ControlStreamer {
        /** transmitComplete must never be called on inbound stream */
        void transmitComplete() final {}
    };

    /** Outbound control streamer with default method implementations */
    struct OutboundControlStreamer: public ControlStreamer {
        /** packetReceived must never be called on outbound stream */
        void packetReceived(const uint8_t *buffer, uint32_t length) final {}

        void completed() override {}
        void aborted() override {}
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_CONTROL_H
