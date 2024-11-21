#include <ub/usb-device-serial.hpp>
#include <ub/usbd/serial-protocol.hpp>

#include <cstring>
#include <algorithm>

using namespace ub::usbd;
using namespace ub::usbd::serial;

constexpr static uint8_t EP_STATUS_IN   = 0;
constexpr static uint8_t EP_DATA_IN     = 1;
constexpr static uint8_t EP_DATA_OUT    = 2;

struct ControlReq {
    constexpr static uint8_t SET_LINE_CODING         = 0x20;
    constexpr static uint8_t GET_LINE_CODING         = 0x21;
    constexpr static uint8_t SET_CONTROL_LINE_STATE  = 0x22;
};

static_assert(sizeof(LineCoding) == LineCoding::LENGTH);

void SerialFunction::discardReceived(size_t length) {
    if (m_rxQueue.pendingBytes() < length) {
        return;
    }

    m_rxQueue.readBytes(length);
    processPendingPacket();
}

size_t SerialFunction::receive(void *buffer, size_t length) {
    size_t queueLength = std::min(length, m_rxQueue.pendingBytes());
    m_rxQueue.readBytes(buffer, queueLength);
    processPendingPacket();
    return queueLength;
}

bool SerialFunction::transmit(const void *buffer, size_t length) {
    if (m_txQueue.freeBytes() < length) {
        return false;
    }

    m_txQueue.writeBytes(buffer, length);

    if (m_txReady) {
        transmitNextChunk();
    }

    return true;
}

#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
uint32_t SerialFunction::functionType() const {
    return 0x229FEE06; // 'microbits.cdc-acm.v1'
}
#endif

FunctionLogic *SerialFunction::initialize(FunctionHost &host, THROWS) {
    m_host = &host;
    m_pendingEvents |= EV_RESET;

    if (m_discardOnReset) {
        m_rxQueue.reset();
        m_txQueue.reset();
        m_rxPacketLength = 0;
        m_txReady = true;
    }

    if (m_rxPacketLength == 0) {
        m_host->receivePacket(EP_DATA_OUT, m_rxPacket);
    }

    transmitNextChunk();
    return this;
}

void SerialFunction::setupControl(ControlRequest &request) {
    if (request.setup->type() != SetupType::CLASS || request.setup->recipient() != SetupRecipient::INTERFACE) {
        return;
    }

    uint8_t setupReq = request.setup->bRequest;
    switch (setupReq) {
    case ControlReq::SET_CONTROL_LINE_STATE:
        request.accepted = true;
        break;

    case ControlReq::GET_LINE_CODING:
    case ControlReq::SET_LINE_CODING:
        request.accepted = true;
        request.maxLength = LineCoding::LENGTH;
        request.direction = ((setupReq & 1) != 0) ? ControlDirection::OUTBOUND : ControlDirection::INBOUND;
        break;

    default:
        return;
    }
}

void SerialFunction::handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) {
    switch (setup.bRequest) {
    case ControlReq::SET_CONTROL_LINE_STATE:
        m_controlSignals = setup.wValue;
        m_pendingEvents |= EV_CONTROL_SIGNALS_CHANGED;
        break;

    case ControlReq::GET_LINE_CODING:
        std::memcpy(buffer, &m_lineCoding, LineCoding::LENGTH);
        length = LineCoding::LENGTH;
        break;

    case ControlReq::SET_LINE_CODING:
        std::memcpy(&m_lineCoding, buffer, LineCoding::LENGTH);
        m_pendingEvents |= EV_LINE_CODING_CHANGED;
        break;
    }
}

void SerialFunction::packetReceived(uint8_t endpoint, size_t length) {
    if (length == 0) {
        m_host->receivePacket(EP_DATA_OUT, m_rxPacket);
        return;
    }

    if (m_rxQueue.freeBytes() >= length) {
        m_rxQueue.writeBytes(m_rxPacket, length);
        m_pendingEvents |= EV_DATA_RX;

        m_host->receivePacket(EP_DATA_OUT, m_rxPacket);
    } else {
        m_rxPacketLength = length;
    }
}

void SerialFunction::transmitComplete(uint8_t endpoint) {
    m_txQueue.readBytes(m_txPacketLength);
    m_pendingEvents |= EV_DATA_TX;
    transmitNextChunk();
}

void SerialFunction::transmitNextChunk() {
    if (m_txQueue.pendingBytes() != 0) {
        m_txPacketLength = std::min(m_txQueue.readLimit(), (size_t) UB_USBD_SERIAL_PACKET_LENGTH);
        m_host->transmitPacket(EP_DATA_IN, m_txQueue.readPtr(), m_txPacketLength);
        m_txReady = false;
    } else {
        m_txPacketLength = 0;
        m_txReady = true;
    }
}

void SerialFunction::processPendingPacket() {
    if (m_rxPacketLength != 0 && m_rxQueue.freeBytes() >= m_rxPacketLength) {
        m_rxQueue.writeBytes(m_rxPacket, m_rxPacketLength);
        m_rxPacketLength = 0;
        m_host->receivePacket(EP_DATA_OUT, m_rxPacket);
    }
}

uint32_t SerialFunction::pullEvents() {
    uint32_t r = m_pendingEvents;
    m_pendingEvents = 0;
    return r;
}
