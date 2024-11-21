#include <ub/usbd/static-config.hpp>
#include <ub/usb-device.hpp>

#include <ub/usbd/impl/control-std.hpp>

#include <algorithm>
#include <cstring>

using namespace ub::usbd;
using namespace ub::usbd::impl;

struct Descriptor {
    constexpr static uint8_t DEVICE             = 0x01; //! Device descriptor type
    constexpr static uint8_t CONFIGURATION      = 0x02; //! Configuration descriptor type
    constexpr static uint8_t STRING             = 0x03; //! String descriptor type
    constexpr static uint8_t DEV_QUALIFIER      = 0x06; //! Device qualifier descriptor type
    constexpr static uint8_t OTHER_SPEED        = 0x07; //! Other speed configuration descriptor type
    constexpr static uint8_t INTERFACE_PWR      = 0x08; //! Interface power descriptor type
};

struct SetupReq {
    constexpr static uint8_t GET_STATUS         = 0x00; //! Get status of the targeted entity
    constexpr static uint8_t CLEAR_FEATURE      = 0x01; //! Clear or disable specific feature of targeted entity
    constexpr static uint8_t SET_FEATURE        = 0x03; //! Set or enable specific feature of targeted entity
    constexpr static uint8_t SET_ADDRESS        = 0x05; //! Set device bus address
    constexpr static uint8_t GET_DESCRIPTOR     = 0x06; //! Get a descriptor from device
    constexpr static uint8_t GET_CONFIGURATION  = 0x08; //! Get index of currently active configuration
    constexpr static uint8_t SET_CONFIGURATION  = 0x09; //! Select a configuration
    constexpr static uint8_t GET_INTERFACE      = 0x0A; //! Get alternative settings of the interface
    constexpr static uint8_t SET_INTERFACE      = 0x0B; //! Select an alternative interface setting
    constexpr static uint8_t SYNCH_FRAME        = 0x0C; //! Get isochronous frame reference number
};

struct FeatureSelector {
    constexpr static uint8_t ENDPOINT_HALT      = 0x01; //! Whether value it stalled
};

void ByteStreamer::transmitComplete() {
    size_t len = std::min(length, endpoint->packetLength());
    uint8_t *buf = endpoint->packetBuffer();

    std::memcpy(buf, data, len);
    data += len;
    length -= len;

    endpoint->transmitPacket(buf, len);
}

void StringStreamer::transmitComplete() {
    constexpr static uint32_t HEADER_LEN = 2;

    uint8_t *buf = endpoint->packetBuffer();
    uint8_t *start = buf;
    size_t len = endpoint->packetLength();

    if (first) {
        buf[0] = HEADER_LEN + 2 * length;
        buf[1] = Descriptor::STRING;

        first = false;
        buf += HEADER_LEN;
        len -= HEADER_LEN;
    }

    while (len > 1) {
        buf[0] = *str++;
        buf[1] = 0;

        buf += 2;
        len -= 2;
    }

    endpoint->transmitPacket(start, buf - start);
}

void ByteStreamer::setRegularDescriptor(const uint8_t *d) {
    data = d;
    length = *d;    // read bLength byte of the descriptor structure
}

void ByteStreamer::setConfigDescriptor(const uint8_t *d) {
    data = d;
    length = *((uint16_t *) (d + 2)); // read wTotalLength byte of configuration descriptor
}

void StandardControlHandler::setupControl(ControlRequest &request) {
    m_func = ControlFunc::DEFAULT;

    switch (request.setup->bRequest) {
    case SetupReq::GET_DESCRIPTOR:
        setupGetDescriptor(request);
        break;

    case SetupReq::SET_ADDRESS:
        request.accepted = true;
        m_func = ControlFunc::SET_ADDRESS;
        break;

    case SetupReq::SET_CONFIGURATION:
    case SetupReq::SET_INTERFACE:
        request.accepted = true;
        break;

    case SetupReq::SET_FEATURE:
    case SetupReq::CLEAR_FEATURE:
        setupFeature(request);
        break;

    case SetupReq::GET_CONFIGURATION:
    case SetupReq::GET_INTERFACE:
        request.accepted = true;
        request.direction = ControlDirection::OUTBOUND;
        request.maxLength = 1;
        break;

    case SetupReq::GET_STATUS:
        request.accepted = true;
        request.direction = ControlDirection::OUTBOUND;
        request.maxLength = 2;
        m_func = ControlFunc::GET_STATUS;
        break;

    default:
        // do nothing - request will be rejected by default
#if UB_USBD_TRAP_UNKNOWN_CONTROL_REQUESTS
        asm volatile ("bkpt");
#endif
        break;
    }
}

void StandardControlHandler::handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) {
    switch (m_func) {
    case ControlFunc::DEFAULT:
        handleDefaultControl(setup, buffer, length, CHECK);
        break;

    case ControlFunc::SET_ADDRESS:
        m_device->m_pcd->setAddress(setup.wValue, PeripheralController::SetAddressPhase::SETUP_RECEIVED);
        break;

    case ControlFunc::SET_STALL:
        m_device->m_pcd->stallEndpoint(setup.wIndex, setup.bRequest == SetupReq::SET_FEATURE);
        break;

    case ControlFunc::GET_STATUS:
        handleGetStatus(setup, buffer, length, CHECK);
        break;
    }
}

void StandardControlHandler::completeControl(const SetupPacket &setup, bool success) {
    if (m_func == ControlFunc::SET_ADDRESS && success) {
        m_device->m_pcd->setAddress(setup.wValue, PeripheralController::SetAddressPhase::STATUS_ACKNOWLEDGED);
    }
}

void StandardControlHandler::setupGetDescriptor(ControlRequest &request) {
    uint8_t type  = request.setup->wValue >> 8;
    uint8_t index = request.setup->wValue;

    new (&m_streamer.byte) ByteStreamer();
    m_streamer.byte.endpoint = request.endpoint;
    request.streamer = &m_streamer.byte;

    request.direction = ControlDirection::OUTBOUND;
    request.maxLength = UINT32_MAX;

    switch (type) {
    case Descriptor::DEVICE:
        request.accepted = true;
        m_streamer.byte.setRegularDescriptor(m_device->m_cfg->descriptors.deviceDescriptor);
        break;

    case Descriptor::CONFIGURATION: {
        request.accepted = true;
        uint8_t speedIndex = linkSpeedIndex(request.endpoint->linkSpeed());
        m_streamer.byte.setConfigDescriptor(m_device->m_cfg->descriptors.configDescriptor[speedIndex]);
        break;
    }

    case Descriptor::STRING:
        setupStringDescriptor(request, index);
        break;

    case Descriptor::DEV_QUALIFIER:
    case Descriptor::OTHER_SPEED:
        return;

    default:
        // do nothing - request will be rejected by default
#if UB_USBD_TRAP_UNKNOWN_CONTROL_REQUESTS
        asm volatile ("bkpt");
#endif
        break;
    }
}

void StandardControlHandler::setupStringDescriptor(ControlRequest &request, uint8_t index) {
    const config::StringDescriptor *str = m_device->m_cfg->descriptors.strings;
    size_t stringCount = m_device->m_cfg->descriptors.stringCount;

#if UB_USBD_RUNTIME_SERIAL_NUMBER
    uint8_t serialIndex = m_device->m_cfg->descriptors.serialNumberIndex;
    if (serialIndex != 0 && index == serialIndex) {
        request.accepted = true;

        new (&m_streamer.str) StringStreamer();
        request.streamer = &m_streamer.str;
        m_streamer.str.endpoint = request.endpoint;
        m_streamer.str.str = m_device->m_serialNumber;
        m_streamer.str.length = strlen(m_device->m_serialNumber);
        m_streamer.str.first = true;

        return;
    }
#endif

    for (size_t i = 0; i < stringCount; i++) {
        if (str[i].index != index) {
            continue;
        }

        request.accepted = true;
        m_streamer.byte.setRegularDescriptor(str[i].data);
        return;
    }
}

void StandardControlHandler::setupFeature(ControlRequest &request) {
    const SetupPacket *s = request.setup;

    if (s->recipient() == SetupRecipient::ENDPOINT && s->wValue == FeatureSelector::ENDPOINT_HALT) {
        if (!m_device->validateEndpoint(s->wIndex)) {
            return;
        }

        m_func = ControlFunc::SET_STALL;
        request.accepted = true;
        return;
    }
}

void StandardControlHandler::handleDefaultControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) {
    uint8_t req = setup.bRequest;

    if (req == SetupReq::SET_CONFIGURATION) {
        using CS = USBDevice::CfgState;
        uint8_t config = setup.wValue; // lower byte only, upper byte is reserved

        if (config != 0) {
            if (m_device->m_configured == CS::RESET) {
                m_device->setConfigured(CHECK);
            }

            m_device->m_configured = CS::CONFIGURED;
        } else if (m_device->m_configured == CS::CONFIGURED) {
            m_device->m_configured = CS::DECONFIGURED;
        }

        return;
    }

    if (req == SetupReq::GET_CONFIGURATION) {
        buffer[0] = (uint8_t) m_device->m_configured;
        length = 1;
        return;
    }

    if (req == SetupReq::GET_INTERFACE) {
        buffer[0] = 0;
        length = 1;
        return;
    }
}

void StandardControlHandler::handleGetStatus(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) {
    buffer[0] = 0;
    buffer[1] = 0;
    length = 2;

    switch (setup.recipient()) {
    case SetupRecipient::DEVICE:
        buffer[0] = 0x01;
        break;

    case SetupRecipient::ENDPOINT:
        if (m_device->validateEndpoint(setup.wIndex)) {
            buffer[0] = m_device->m_pcd->stalled(setup.wIndex);
        }
        break;

    default:
        break;
    }
}
