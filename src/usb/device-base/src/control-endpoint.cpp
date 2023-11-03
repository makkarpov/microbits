#include <ub/usbd/impl/control-endpoint.hpp>
#include <ub/usb-device.hpp>

#include <cstring>
#include <algorithm>

using namespace ub::usbd;
using namespace ub::usbd::impl;

void ControlRequest::reset() {
    accepted    = false;
    // direction is always INBOUND here. Handler must change it to OUTBOUND if it wishes to send data.
    // actual direction will be validated when request is processed
    direction   = ControlDirection::INBOUND;
    maxLength   = 0;
    streamer    = nullptr;
}

void ControlEndpointImpl::reset(LinkSpeed speed) {
    m_speed = speed;
    m_state = SetupState::IDLE;

    if (m_streamer != nullptr) {
        m_streamer->aborted();
        m_streamer = nullptr;
    }

    m_pcd->receivePacket(EP_CONTROL_OUT, m_buffer);
}

void ControlEndpointImpl::packetReceived(uint32_t length, bool setup) {
    STATUS_CODE_DEFN;

    if (setup) {
        setupReceived(length, STATUS_CODE);
    } else {
        packetReceived(length, STATUS_CODE);
    }

    if (HAS_PENDING_ERROR) {
        abortRequest();
    }
}

void ControlEndpointImpl::setupReceived(uint32_t length, THROWS) {
    if (m_streamer != nullptr) {
        m_streamer->aborted();
        m_streamer = nullptr;
    } else if (m_handler != nullptr) {
        m_handler->completeControl(m_setup, false);
    }

    m_handler = nullptr;
    m_state = SetupState::IDLE;
    m_dataLength = 0;

    if (length != SetupPacket::LENGTH) {
        THROW(usbError, E_INVALID_SETUP_LENGTH);
    }

    std::memcpy(&m_setup, m_buffer, SetupPacket::LENGTH);

    ControlRequest request {};
    request.setup       = &m_setup;
    request.endpoint    = this;
    request.reset();

    m_handler = m_device->resolveControl(request, m_setup);
    if (m_handler == nullptr) {
        THROW(usbError, E_UNRESOLVED_CONTROL_REQUEST);
    }

    if (!request.accepted) {
        m_handler->setupControl(request);
    }

    if (!request.accepted) {
        THROW(usbError, E_CONTROL_REQUEST_REJECTED);
    }

    // set the streamer field here, so it will always be aborted if further processing fails
    m_streamer = request.streamer;

    bool deviceToHost = request.direction == ControlDirection::OUTBOUND;
    bool valid = m_setup.deviceToHost() == deviceToHost && m_setup.wLength <= request.maxLength;

    if (valid && m_streamer == nullptr) {
        // One-shot requests must fit into a single packet:
        valid = m_setup.wLength <= UB_USBD_MAX_CONTROL_PACKET;
    }

    if (!valid) {
        THROW(usbError, E_CONTROL_VALIDATION_FAILED);
    }

    if (deviceToHost) {
        if (m_streamer != nullptr) {
            m_state = SetupState::WAITING;
            m_packetLength = UB_USBD_MAX_CONTROL_PACKET;
            m_streamer->transmitComplete();
        } else {
            uint32_t responseLength = UB_USBD_MAX_CONTROL_PACKET;
            m_handler->handleControl(m_setup, m_buffer, responseLength, CHECK);

            if (responseLength > m_setup.wLength) {
                responseLength = m_setup.wLength;
            }

            m_pcd->transmitPacket(EP_CONTROL_IN, m_buffer, responseLength);
            m_state = SetupState::TX_DATA;
        }
    } else if (m_setup.wLength == 0) {
        if (m_streamer != nullptr) {
            streamReceiveNext(0);

            if (m_state != SetupState::IDLE) {
                m_state = SetupState::TX_STATUS;
                m_pcd->transmitPacket(EP_CONTROL_IN, m_buffer, 0);
            }
        } else {
            uint32_t requestLength = 0;
            m_handler->handleControl(m_setup, m_buffer, requestLength, CHECK);
            m_state = SetupState::TX_STATUS;
            m_pcd->transmitPacket(EP_CONTROL_IN, m_buffer, 0);
        }
    } else {
        m_state = SetupState::RX_DATA;
        m_pcd->receivePacket(EP_CONTROL_OUT, m_buffer);
    }
}

void ControlEndpointImpl::packetReceived(uint32_t length, ub::Status &_statusCode) {
    if (m_state == SetupState::RX_STATUS) {
        completeRequest();
        return;
    }

    if (m_state == SetupState::RX_DATA) {
        bool completed;

        if (m_streamer != nullptr) {
            if (m_dataLength + length > m_setup.wLength) {
                THROW(genericError, E_CONTROL_DATA_TOO_LONG);
            }

            completed = streamReceiveNext(length);
        } else {
            uint32_t requestLength = length;

            if (requestLength > m_setup.wLength) {
                requestLength = m_setup.wLength;
            }

            m_handler->handleControl(m_setup, m_buffer, requestLength, CHECK);
            completed = true;
        }

        if (completed) {
            m_state = SetupState::TX_STATUS;
            m_pcd->transmitPacket(EP_CONTROL_IN, m_buffer, 0);
        }

        return;
    }
}

void ControlEndpointImpl::transmitComplete() {
    if (m_state == SetupState::TX_STATUS) {
        completeRequest();
        return;
    }

    if (m_state == SetupState::TX_DATA) {
        m_state = SetupState::RX_STATUS;
        m_pcd->receivePacket(EP_CONTROL_OUT, m_buffer);
        return;
    }

    if (m_state == SetupState::TX_MORE_DATA) {
        // m_streamer is not null, otherwise this state cannot occur
        m_state = SetupState::WAITING;
        m_packetLength = UB_USBD_MAX_CONTROL_PACKET;
        m_streamer->transmitComplete();
        return;
    }
}

void ControlEndpointImpl::receivePacket() {
    m_pcd->receivePacket(EP_CONTROL_OUT, m_buffer);
    m_state = SetupState::RX_DATA;
}

void ControlEndpointImpl::transmitPacket(const void *buffer, size_t length) {
    length = std::min(length, (size_t) (m_setup.wLength - m_dataLength));
    m_pcd->transmitPacket(EP_CONTROL_IN, buffer, length);

    m_dataLength += length;

    if (m_dataLength == m_setup.wLength || length < UB_USBD_MAX_CONTROL_PACKET) {
        m_state = SetupState::TX_DATA;
    } else {
        m_state = SetupState::TX_MORE_DATA;
    }
}

void ControlEndpointImpl::abortRequest() {
    m_state = SetupState::IDLE;

    m_pcd->stallEndpoint(EP_CONTROL_IN, true);
    m_pcd->stallEndpoint(EP_CONTROL_OUT, true);

    if (m_streamer != nullptr) {
        m_streamer->aborted();
        m_streamer = nullptr;
    } else if (m_handler != nullptr) {
        m_handler->completeControl(m_setup, false);
    }

    m_handler = nullptr;
}

bool ControlEndpointImpl::streamReceiveNext(uint32_t length) {
    m_state = SetupState::WAITING;
    m_dataLength += length;
    m_streamer->packetReceived(m_buffer, length);
    return m_state != SetupState::IDLE && m_dataLength >= m_setup.wLength;
}

void ControlEndpointImpl::completeRequest() {
    if (m_streamer != nullptr) {
        m_streamer->completed();
        m_streamer = nullptr;
    } else {
        m_handler->completeControl(m_setup, true);
    }

    m_handler = nullptr;
    m_state = SetupState::IDLE;
    m_pcd->receivePacket(EP_CONTROL_OUT, m_buffer);
}
