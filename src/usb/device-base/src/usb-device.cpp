#include <ub/usb-device.hpp>

#include <ub/usbd/impl/endpoint-mapper.hpp>

using namespace ub::usbd;

void USBDevice::registerFunction(Function &fn) {
    if (m_funcIdx == UB_USBD_MAX_FUNCTIONS) {
        return;
    }

    m_func[m_funcIdx] = &fn;
    m_funcIdx++;
}

void USBDevice::initialize(PeripheralController &peripheral, const StaticDescriptor &descriptor, THROWS) {
    m_pcd = &peripheral;
    m_desc = &descriptor;

    if (m_funcIdx != descriptor.functionCount) {
        THROW(usbError, E_FUNCTION_MISMATCH);
    }

    for (size_t i = 0; i < m_funcIdx; i++) {
        if (m_func[i]->functionType() != m_desc->functions[i].functionType) {
            THROW(usbError, E_FUNCTION_MISMATCH);
        }

        m_funcHosts[i].dev = this;
    }

    m_pcd->initialize(CHECK);

    m_control.m_pcd = m_pcd;
    m_control.m_device = this;
    m_ctrlHandler.m_device = this;
}

void USBDevice::start() {
    m_pcd->connect();
}

void USBDevice::stop() {
    m_pcd->disconnect();
    m_control.reset(LinkSpeed::NONE);
}

uint32_t USBDevice::processEvents() {
    PeripheralEvent ev {};
    uint32_t ret = 0;

    while (m_pcd->pullEvent(ev)) {
        switch (ev.t) {
        case PeripheralEvent::EV_RESET:
            ret |= EV_RESET;
            processReset(ev.speed);
            break;

        case PeripheralEvent::EV_PACKET_RECEIVED:
            processPacketReceived(ev.packet);
            break;

        case PeripheralEvent::EV_TRANSMIT_COMPLETE:
            processTransmitComplete(ev.addr);
            break;
        }
    }

    return ret;
}

void USBDevice::processReset(LinkSpeed speed) {
    m_control.reset(speed);
    m_configured = CfgState::RESET;
}

void USBDevice::processPacketReceived(const PeripheralEvent::RxPacket &ev) {
    if (ev.setup || ev.addr == EP_CONTROL_OUT) {
        m_control.packetReceived(ev.size, ev.setup);
        return;
    }

#if UB_USBD_HAVE_DATA_ENDPOINTS
    auto logical = impl::toLogicalEndpoint(ev.addr, m_desc->endpointMapping);

    if (logical) {
        m_funcLogic[logical.function()]->packetReceived(logical.value(), ev.size);
    }
#endif
}

void USBDevice::processTransmitComplete(uint8_t endpoint) {
    if (endpoint == EP_CONTROL_IN) {
        m_control.transmitComplete();
        return;
    }

#if UB_USBD_HAVE_DATA_ENDPOINTS
    auto logical = impl::toLogicalEndpoint(endpoint, m_desc->endpointMapping);

    if (logical) {
        m_funcLogic[logical.function()]->transmitComplete(logical.value());
    }
#endif
}

ControlHandler *USBDevice::resolveControl(ControlRequest &req, SetupPacket &setup) {
    if (setup.recipient() == SetupRecipient::ENDPOINT && !validateEndpoint(setup.wIndex)) {
        return nullptr;
    }

    if (setup.type() == SetupType::STANDARD) {
        return &m_ctrlHandler;
    }

    if (m_configured != CfgState::CONFIGURED) {
        return nullptr;
    }

    if (setup.recipient() == SetupRecipient::DEVICE) {
#if UB_USBD_HAVE_DATA_ENDPOINTS
        for (size_t i = 0; i < m_funcIdx; i++) {
            m_funcLogic[i]->setupControl(req);
            if (req.accepted) {
                return m_funcLogic[i];
            }

            req.reset();
        }

        return nullptr;
#else
        return m_funcLogic[0];
#endif
    }

    if (setup.recipient() == SetupRecipient::ENDPOINT) {
#if UB_USBD_HAVE_DATA_ENDPOINTS
        auto logical = impl::toLogicalEndpoint(req.setup->wIndex, m_desc->endpointMapping);

        if (logical) {
            setup.wIndex = logical.value();
            return m_funcLogic[logical.function()];
        }
#endif

        return nullptr;
    }

    if (setup.recipient() == SetupRecipient::INTERFACE) {
#if UB_USBD_HAVE_DATA_ENDPOINTS
        auto logical = impl::toLogicalInterface(req.setup->wIndex, m_desc->endpointMapping);

        if (logical) {
            setup.wIndex = logical.value();
            return m_funcLogic[logical.function()];
        }

        return nullptr;
#else
        return m_funcLogic[0];
#endif
    }

    return nullptr;
}

void USBDevice::setConfigured(THROWS) {
#if UB_USBD_HAVE_DATA_ENDPOINTS
    for (size_t i = 0; i < m_desc->endpointCount; i++) {
        m_pcd->openEndpoint(m_desc->endpoints[i], CHECK);
    }
#endif

    for (size_t i = 0; i < m_funcIdx; i++) {
        m_funcLogic[i] = m_func[i]->initialize(m_funcHosts[i], CHECK);
    }
}

bool USBDevice::validateEndpoint(uint32_t endpoint) {
    if ((endpoint & EP_NUM) == 0) {
        return true;
    }

    if ((endpoint & 0xFFFFFF00) != 0) {
        return false;
    }

#if UB_USBD_HAVE_DATA_ENDPOINTS
    return (bool) impl::toLogicalEndpoint(endpoint, m_desc->endpointMapping);
#else
    return false; // control endpoint is already covered, no data endpoints present
#endif
}

LinkSpeed USBDevice::FnHostImpl::linkSpeed() const {
    return dev->m_control.m_speed;
}

void USBDevice::FnHostImpl::stallEndpoint(uint8_t endpoint, bool stall) {
    uint8_t physical = impl::toPhysicalEndpoint(endpoint, functionDesc());
    // TODO - check validity
    dev->m_pcd->stallEndpoint(physical, stall);
}

bool USBDevice::FnHostImpl::stalled(uint8_t endpoint) {
    uint8_t physical = impl::toPhysicalEndpoint(endpoint, functionDesc());
    return dev->m_pcd->stalled(physical);
}

void USBDevice::FnHostImpl::receivePacket(uint8_t endpoint, void *buffer) {
    uint8_t physical = impl::toPhysicalEndpoint(endpoint, functionDesc());
    dev->m_pcd->receivePacket(physical, buffer);
}

void USBDevice::FnHostImpl::transmitPacket(uint8_t endpoint, const void *buffer, uint32_t length) {
    uint8_t physical = impl::toPhysicalEndpoint(endpoint, functionDesc());
    dev->m_pcd->transmitPacket(physical, buffer, length);
}
