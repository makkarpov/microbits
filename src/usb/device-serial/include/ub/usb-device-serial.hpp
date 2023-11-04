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
            m_rxPacketLength(0), m_txPacketLength(0), m_txReady(false), m_controlSignals(0)
        {}

        /** Set buffer to be used as a receive queue. Length must be sufficient to hold single packet. */
        void setReceiveBuffer(void *buffer, size_t length) { m_rxQueue.setBuffer(buffer, length); }

        /** Set buffer to be used as a transmit buffer */
        void setTransmitBuffer(void *buffer, size_t length) { m_txQueue.setBuffer(buffer, length); }

        /** @return Number of pending received bytes */
        [[nodiscard]] size_t receivePendingBytes() const { return m_rxQueue.pendingBytes() + m_rxPacketLength; }

        /** @return Direct reference to receive queue for zero-copy data processing */
        [[nodiscard]] const ub::CircularBuffer &receiveQueue() const { return m_rxQueue; }

        /** Discard first `length` received bytes. Does nothing if queue contains less pending bytes than specified. */
        void discardReceived(size_t length);

        /** Receive next data chunk. Returns number of bytes actually received. */
        size_t receive(void *buffer, size_t length);

        /** Transmit next data chunk. Fails if free buffer space is insufficient to hold the message */
        bool transmit(const void *buffer, size_t length);

        /** @return Free space in the transmit queue */
        [[nodiscard]] size_t transmitFreeBytes() const { return m_txQueue.freeBytes(); }

        /** Reference to a line coding structure, specifying UART parameters */
        [[nodiscard]] serial::LineCoding &lineCoding() { return m_lineCoding; }

        /** Currently active control signals */
        [[nodiscard]] uint16_t controlSignals() const { return m_controlSignals; }

        // Internal methods:
        [[nodiscard]] uint32_t functionType() const override;
        FunctionLogic* initialize(FunctionHost &host, THROWS) override;

    private:
        FunctionHost        *m_host;

        ub::CircularBuffer  m_txQueue;
        ub::CircularBuffer  m_rxQueue;
        uint8_t             m_rxPacket[UB_USBD_SERIAL_PACKET_LENGTH];
        size_t              m_rxPacketLength;
        size_t              m_txPacketLength;
        bool                m_txReady;

        serial::LineCoding  m_lineCoding;
        uint16_t            m_controlSignals;

        void setupControl(ControlRequest &request) override;
        void handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, Status &_statusCode) override;
        void packetReceived(uint8_t endpoint, size_t length) override;
        void transmitComplete(uint8_t endpoint) override;

        void transmitNextChunk();
        void processPendingPacket();
    };
}

#endif // UB_USB_DEVICE_SERIAL_USB_DEVICE_SERIAL_H
