#include <ub/usb-device.hpp>

#include <ub/usbd/impl/endpoint-mapper.hpp>

using namespace ub::usbd;

void USBDevice::registerFunction(Function &fn) {
    if (m_nextFunctionIdx == UB_USBD_MAX_FUNCTIONS) {
        return;
    }

    m_funcs[m_nextFunctionIdx].func = &fn;
    m_nextFunctionIdx++;
}

void USBDevice::initialize(PeripheralController &peripheral, const StaticConfig &config, THROWS) {
    m_pcd = &peripheral;
    m_cfg = &config;

    if (m_nextFunctionIdx != config.functionCount) {
        THROW(usbError, E_FUNCTION_MISMATCH);
    }

    for (size_t i = 0; i < m_nextFunctionIdx; i++) {
        auto &f = m_funcs[i];

#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
        if (f.func->functionType() != m_cfg->functions[i].functionTypeId) {
            THROW(usbError, E_FUNCTION_MISMATCH);
        }
#endif

        f.dev = this;
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

#if UB_USBD_HAVE_RESOURCE_MAPPING
    m_activeMapping = &m_cfg->mapping[impl::linkSpeedIndex(speed)];
#endif
}

void USBDevice::processPacketReceived(const PeripheralEvent::RxPacket &ev) {
    if (ev.setup || ev.addr == EP_CONTROL_OUT) {
        m_control.packetReceived(ev.size, ev.setup);
        return;
    }

#if UB_USBD_HAVE_DATA_ENDPOINTS
    auto logical = impl::toLogicalEndpoint(ev.addr, *m_activeMapping);

    if (logical) {
        m_funcs[logical.function()].logic->packetReceived(logical.value(), ev.size);
    }
#endif
}

void USBDevice::processTransmitComplete(uint8_t endpoint) {
    if (endpoint == EP_CONTROL_IN) {
        m_control.transmitComplete();
        return;
    }

#if UB_USBD_HAVE_DATA_ENDPOINTS
    auto logical = impl::toLogicalEndpoint(endpoint, *m_activeMapping);

    if (logical) {
        m_funcs[logical.function()].logic->transmitComplete(logical.value());
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
        for (size_t i = 0; i < m_nextFunctionIdx; i++) {
            auto logic = m_funcs[i].logic;

            logic->setupControl(req);
            if (req.accepted) {
                return logic;
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
        auto logical = impl::toLogicalEndpoint(req.setup->wIndex, *m_activeMapping);

        if (logical) {
            setup.wIndex = logical.value();
            return m_funcs[logical.function()].logic;
        }
#endif

        return nullptr;
    }

    if (setup.recipient() == SetupRecipient::INTERFACE) {
#if UB_USBD_HAVE_DATA_ENDPOINTS
        auto logical = impl::toLogicalInterface(req.setup->wIndex, *m_activeMapping);

        if (logical) {
            setup.wIndex = logical.value();
            return m_funcs[logical.function()].logic;
        }

        return nullptr;
#else
        return m_funcLogic[0];
#endif
    }

    return nullptr;
}

void USBDevice::setConfigured(THROWS) {
    m_pcd->configureDevice(m_cfg->targetData, impl::linkSpeedIndex(m_control.m_speed), CHECK);

    for (size_t i = 0; i < m_nextFunctionIdx; i++) {
        FnHostImpl &f = m_funcs[i];
        f.logic = f.func->initialize(f, CHECK);
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
    return (bool) impl::toLogicalEndpoint(endpoint, *m_activeMapping);
#else
    return false; // control endpoint is already covered, no data endpoints are present
#endif
}

LinkSpeed USBDevice::FnHostImpl::linkSpeed() const {
    return dev->m_control.m_speed;
}

void USBDevice::FnHostImpl::stallEndpoint(uint8_t endpoint, bool stall) {
    dev->m_pcd->stallEndpoint(toPhysicalEndpoint(endpoint), stall);
}

bool USBDevice::FnHostImpl::stalled(uint8_t endpoint) {
    return dev->m_pcd->stalled(toPhysicalEndpoint(endpoint));
}

void USBDevice::FnHostImpl::receivePacket(uint8_t endpoint, void *buffer) {
    dev->m_pcd->receivePacket(toPhysicalEndpoint(endpoint), buffer);
}

void USBDevice::FnHostImpl::transmitPacket(uint8_t endpoint, const void *buffer, uint32_t length) {
    dev->m_pcd->transmitPacket(toPhysicalEndpoint(endpoint), buffer, length);
}

uint8_t USBDevice::FnHostImpl::toPhysicalEndpoint(uint8_t endpoint) const {
#if UB_USBD_HAVE_RESOURCE_MAPPING
    return impl::toPhysicalEndpoint(functionIdx(), endpoint, *dev->m_activeMapping);
#else
    return endpoint;
#endif
}
