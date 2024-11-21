#ifndef UB_USB_DEVICE_SERIAL_USB_DEVICE_SERIAL_H
#define UB_USB_DEVICE_SERIAL_USB_DEVICE_SERIAL_H

#include <ub/usbd/serial-config.hpp>
#include <ub/usbd/serial-protocol.hpp>
#include <ub/usbd/function.hpp>

#include <ub/fifo.hpp>

namespace ub::usbd {
    /**
     * Serial port (CDC ACM) function implementation.
     *
     * This class is NOT thread safe. All calls to this class (as well as calls to other USB stack functions) must be
     * synchronized externally.
     */
    class SerialFunction: public Function, private FunctionLogic {
    public:
        constexpr explicit SerialFunction():
            m_host(nullptr), m_txQueue(), m_rxQueue(), m_rxPacket {},
            m_rxPacketLength(0), m_txPacketLength(0), m_pendingEvents(0), m_controlSignals(0),
            m_txReady(false), m_discardOnReset(true)
        {}

        /** Set buffer to be used as a receive queue. Length must be sufficient to hold single packet. */
        void setReceiveBuffer(void *buffer, size_t length) { m_rxQueue.setBuffer(buffer, length); }

        /** Set buffer to be used as a transmit buffer */
        void setTransmitBuffer(void *buffer, size_t length) { m_txQueue.setBuffer(buffer, length); }

        /**
         * Set whether TX and RX FIFOs should be discarded on the USB reset event.
         *
         * When false (default), FIFOs retain their state and data on USB reset. Any pending TX data is immediately
         * scheduled for the transmission.
         *
         * When true, all pending data is discarded and all byte counters are reset to zero.
         */
        void setDiscardOnReset(bool value) { m_discardOnReset = value; }

        /** @return Total number of received bytes still waiting in the queue */
        [[nodiscard]] size_t receivePendingBytes() const { return m_rxQueue.pendingBytes() + m_rxPacketLength; }

        /**
         * Returns total number of received bytes that were already consumed by the application.
         *
         * Together with `receiveTailPos` these methods provide an useful mechanism to track the position of
         * out-of-band events (such as baud rate change or UART BREAKs) relative to the byte stream.
         */
        [[nodiscard]] size_t receiveHeadPos() const { return m_rxQueue.headPosition(); }

        /**
         * Returns total number of received bytes, including those that are still waiting in the queue.
         *
         * Note that this method must be used instead of just `receiveQueue().tailPosition()` because it accounts for
         * the buffered packet length.
         *
         * Together with `receiveHeadPos` these methods provide an useful mechanism to track the position of
         * out-of-band events (such as baud rate change or UART BREAKs) relative to the byte stream.
         */
        [[nodiscard]] size_t receiveTailPos() const { return m_rxQueue.tailPosition() + m_rxPacketLength; }

        /**
         * Returns direct reference to the receive queue for zero-copy data processing
         *
         * Don't use this reference to query number of pending bytes and absolute stream positions. Use
         * `receivePendingBytes()`, `receiveHeadPos()` and `receiveTailPos()` methods instead.
         */
        [[nodiscard]] const ub::CircularBuffer &receiveQueue() const { return m_rxQueue; }

        /**
         * Discard first `length` received bytes and advance the receiver state machine. Does nothing if queue contains
         * less pending bytes than specified.
         */
        void discardReceived(size_t length);

        /** Receive next data chunk. Returns number of bytes actually received. */
        size_t receive(void *buffer, size_t length);

        /** Transmit next data chunk. Fails if free buffer space is insufficient to hold the message */
        bool transmit(const void *buffer, size_t length);

        /** @return Free space in the transmit queue */
        [[nodiscard]] size_t transmitFreeBytes() const { return m_txQueue.freeBytes(); }

        /** @return Total number of transmitted bytes that were acknowledged by the host */
        [[nodiscard]] size_t transmitHeadPos() const { return m_txQueue.headPosition(); }

        /** @return Total number of transmitted bytes, including those that are still waiting in the queue. */
        [[nodiscard]] size_t transmitTailPos() const { return m_txQueue.tailPosition(); }

        /** Reference to a line coding structure, specifying UART parameters */
        [[nodiscard]] serial::LineCoding &lineCoding() { return m_lineCoding; }

        /** Currently active control signals */
        [[nodiscard]] uint16_t controlSignals() const { return m_controlSignals; }

        /** Returns and resets a bitmask (see `EV_*` constants) of currently pending events  */
        uint32_t pullEvents();

        // Internal methods:
#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
        [[nodiscard]] uint32_t functionType() const override;
#endif

        FunctionLogic* initialize(FunctionHost &host, THROWS) override;

        /** USB reset has been received from the host */
        constexpr static uint32_t EV_RESET                      = 0x00000001;

        /** Data has been received from the host (RX fifo state changed) */
        constexpr static uint32_t EV_DATA_RX                    = 0x00000002;

        /** Transmitted data has been acknowledged by the host (TX fifo state changed) */
        constexpr static uint32_t EV_DATA_TX                    = 0x00000004;

        /** Line coding parameters (baud rate, number of bits, parity ...) have been changed */
        constexpr static uint32_t EV_LINE_CODING_CHANGED        = 0x00000008;

        /** RS-232 control signals have been changed */
        constexpr static uint32_t EV_CONTROL_SIGNALS_CHANGED    = 0x00000010;

    private:
        FunctionHost        *m_host;

        ub::CircularBuffer  m_txQueue;
        ub::CircularBuffer  m_rxQueue;
        uint8_t             m_rxPacket[UB_USBD_SERIAL_PACKET_LENGTH];
        size_t              m_rxPacketLength;
        size_t              m_txPacketLength;
        uint32_t            m_pendingEvents;

        serial::LineCoding  m_lineCoding;
        uint16_t            m_controlSignals;
        bool                m_txReady;
        bool                m_discardOnReset;

        void setupControl(ControlRequest &request) override;
        void handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, Status &_statusCode) override;
        void packetReceived(uint8_t endpoint, size_t length) override;
        void transmitComplete(uint8_t endpoint) override;

        void transmitNextChunk();
        void processPendingPacket();
    };
}

#endif // UB_USB_DEVICE_SERIAL_USB_DEVICE_SERIAL_H
