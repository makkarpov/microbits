#include "pcd_fs_devices.hpp"

#if defined(UB_USBD_FS_CORE)

#include "pcd_fs.hpp"

#include <ub/stm32/usb-device.hpp>
#include <ub/usbd/static-config.hpp>

using namespace ub::usbd;
using namespace ub::stm32::usb;
using namespace ub::stm32::usb::impl;

enum {
    // Bits to keep and to unconditionally set when a "toggle" write to EPnR is expected
    USB_EPnR_MASK  = USB_R_EPnR_ADDR | USB_R_EPnR_EP_KIND | USB_R_EPnR_TYPE,
    USB_EPnR_SET   = USB_R_EPnR_CTR_TX | USB_R_EPnR_CTR_RX
};

// Assumes that length is already aligned to at least half-word
constexpr static size_t usbd_alignReceiveLength(size_t length) {
    return length >= 64 ? (length + 31) & -32 : length;
}

constexpr static uint16_t usbd_encodeReceiveLength(size_t length) {
    return length >= 64
        ? 0x20 | ((length >> 5) - 1)
        : (length >> 1) - 1;
}

static void usbd_setEndpointStatus(uint8_t endpoint, uint8_t status) {
    uint32_t statPos = (endpoint & EP_IN) != 0 ? USB_R_EPnR_STAT_TX_Pos : USB_R_EPnR_STAT_RX_Pos;
    endpoint &= EP_NUM;

    uint32_t epnr = USB_R_EPnR[endpoint];
    uint32_t stat = (epnr >> statPos) & USB_R_EPnR_STAT_Msk;

    USB_R_EPnR[endpoint] = (epnr & USB_EPnR_MASK) | USB_EPnR_SET | ((stat ^ status) << statPos);
}

static void usbd_clearEndpointFlag(uint8_t endpoint, uint32_t flag) {
    uint32_t epnr = USB_R_EPnR[endpoint];
    USB_R_EPnR[endpoint] = ((epnr & USB_EPnR_MASK) | USB_EPnR_SET) & ~flag;
}

void STM32USBPeripheral::initialize(THROWS) {
    // Power-up and reset the transceiver & force CNTR to the known state:
    USB_R_CNTR = USB_R_CNTR_USBRST;

    // Wait for some time as asked by the reference manual
    volatile uint32_t timeout = 100000;
    while (timeout != 0) timeout--;

    USB_R_CNTR = 0; // Release USB reset condition
    USB_R_ISTR = 0; // Clear all pending interrupts

#if defined(USB_R_BTABLE)
    USB_R_BTABLE = 0;
#endif
}

void STM32USBPeripheral::configureDevice(const void *untypedConfigData, uint8_t speedIndex, THROWS) {
    auto configData = (const ub::usbd::config::GenericTargetData *) untypedConfigData;

#if UB_USBD_HAVE_DATA_ENDPOINTS
    const config::EndpointConfig* endpoints = configData->endpoints[speedIndex];
    for (uint32_t i = 0; i < configData->endpointCount[speedIndex]; i++) {
        openEndpoint(endpoints[i], CHECK);
    }
#endif
}

#if UB_USBD_HAVE_DATA_ENDPOINTS
enum {
    // MASK: STAT & DTOG for direction -- resets these back to zero, TOGGLE flips DTOG to NAK
    // TOGGLE: preserve CTR for opposite direction (writing a zero bit clears it), set DTOG to NAK

    // IN (dev -> host), device TX
    EPnR_INIT_IN_MASK       = USB_R_EPnR_STAT_TX | USB_R_EPnR_DTOG_TX,
    EPnR_INIT_IN_TOGGLE     = USB_R_EPnR_CTR_RX | (USB_R_EPnR_STAT_NAK_Val << USB_R_EPnR_STAT_TX_Pos),

    // OUT (host -> dev), device RX
    EPnR_INIT_OUT_MASK      = USB_R_EPnR_STAT_RX | USB_R_EPnR_DTOG_RX,
    EPnR_INIT_OUT_TOGGLE    = USB_R_EPnR_CTR_TX | (USB_R_EPnR_STAT_NAK_Val << USB_R_EPnR_STAT_RX_Pos)
};

static const uint8_t usbd_endpointTypes[4] = {
        USB_R_EPnR_TYPE_CONTROL_Val,
        USB_R_EPnR_TYPE_ISOCHRONOUS_Val,
        USB_R_EPnR_TYPE_BULK_Val,
        USB_R_EPnR_TYPE_INTERRUPT_Val
};

void STM32USBPeripheral::openEndpoint(const ub::usbd::config::EndpointConfig &config, THROWS) {
    uint8_t epNumber = config.address & EP_NUM;
    size_t length = usbd_alignLength(config.maxPacket);
    uint32_t epnr = USB_R_EPnR[epNumber];

    if ((config.address & EP_IN) != 0) {
        epnr = (epnr & EPnR_INIT_IN_MASK) ^ EPnR_INIT_IN_TOGGLE;
        m_in[epNumber].bufferOffset = m_bufferPtr;
        m_in[epNumber].bufferLength = config.maxPacket;
    } else {
        length = usbd_alignReceiveLength(length);
        usbd_setRxDescriptor(USB_BUFFER_TABLE[epNumber], m_bufferPtr, usbd_encodeReceiveLength(length));

        epnr = (epnr & EPnR_INIT_OUT_MASK) ^ EPnR_INIT_OUT_TOGGLE;
        m_out[epNumber].bufferOffset = m_bufferPtr;
    }

    m_bufferPtr += length;
    USB_R_EPnR[epNumber] = epnr
            | (usbd_endpointTypes[(int) config.type] << USB_R_EPnR_TYPE_Pos)
            | (config.address & EP_NUM);
}
#endif

void STM32USBPeripheral::connect() {
    USB_R_CNTR |= USB_R_CNTR_RESETM | USB_R_CNTR_CTRM;
    USB_R_BCDR |= USB_R_BCDR_DPPU;
}

void STM32USBPeripheral::disconnect() {
    USB_R_BCDR &= ~USB_BCDR_DPPU;

    USB_R_CNTR = 0;
    USB_R_DADDR = 0;
    USB_R_ISTR = 0;
}

void STM32USBPeripheral::setAddress(uint8_t address, SetAddressPhase phase) {
    if (phase != SetAddressPhase::STATUS_ACKNOWLEDGED) {
        return;
    }

    USB_R_DADDR = address | USB_R_DADDR_EF;
}

void STM32USBPeripheral::receivePacket(uint8_t endpoint, void *target) {
    m_out[endpoint].target = target;
    usbd_setEndpointStatus(endpoint, USB_R_EPnR_STAT_VALID_Val);
}

void STM32USBPeripheral::transmitPacket(uint8_t endpoint, const void *buffer, size_t length) {
    endpoint &= EP_NUM;

    length = std::min(length, (size_t) m_in[endpoint].bufferLength);
    size_t bufferOffset = m_in[endpoint].bufferOffset;

    usbd_writePacket(USB_MEMORY + bufferOffset, buffer, length);
    usbd_setTxDescriptor(USB_BUFFER_TABLE[endpoint], bufferOffset, length);

    usbd_setEndpointStatus(EP_IN | endpoint, USB_R_EPnR_STAT_VALID_Val);
}

void STM32USBPeripheral::stallEndpoint(uint8_t address, bool stall) {
    usbd_setEndpointStatus(address, stall ? USB_R_EPnR_STAT_STALL_Val : USB_R_EPnR_STAT_NAK_Val);
}

bool STM32USBPeripheral::stalled(uint8_t address) {
    uint16_t epnr = USB_R_EPnR[address & EP_NUM];
    uint32_t statPos = ((address & EP_IN) != 0) ? USB_R_EPnR_STAT_TX_Pos : USB_R_EPnR_STAT_RX_Pos;
    return ((epnr >> statPos) & USB_R_EPnR_STAT_Msk) == USB_R_EPnR_STAT_STALL_Val;
}

#if defined(STM32H5) && UB_STM32_USB_H5_ERRATA_WORKAROUND
static void usbd_errataWorkaround() {
    // ES0565 - software must wait 800ns after receiving CTR interrupt before accessing the SRAM buffer
    // At the worst case of 250 MHz core clock this translates to 200 clock cycles.
    // Routine is written in asm to have a bit more predictable delay (even though it still varies wildly).

    asm volatile (
            "movs   r0, 100         \n"

            "1:                     \n"
            "subs   r0, r0, #1      \n"
            "bne    1b              \n"

            ::: "r0"
    );
}
#endif

bool STM32USBPeripheral::pullEvent(PeripheralEvent &ev) {
    uint32_t status = USB_R_ISTR;

    if ((status & USB_R_ISTR_RESET) != 0) {
        USB_R_ISTR = ~USB_R_ISTR_RESET;

        handleReset();

        ev.t = PeripheralEvent::EV_RESET;
        ev.speed = LinkSpeed::FULL;
        return true;
    }

    if ((status & USB_R_ISTR_CTR) != 0) {
        uint8_t endpoint = status & USB_R_ISTR_EP_ID;

#if defined(STM32H5) && UB_STM32_USB_H5_ERRATA_WORKAROUND
        usbd_errataWorkaround();
#endif

        if ((status & USB_R_ISTR_DIR) != 0) {
            // Data received (OUT transaction)
            ev.t = PeripheralEvent::EV_PACKET_RECEIVED;

            ev.packet.addr = endpoint;
            ev.packet.setup = (USB_R_EPnR[endpoint] & USB_R_EPnR_SETUP) != 0;
            ev.packet.size = usbd_readRxLength(USB_BUFFER_TABLE[endpoint]);

            usbd_readPacket(m_out[endpoint].target, USB_MEMORY + m_out[endpoint].bufferOffset, ev.packet.size);
            usbd_clearEndpointFlag(endpoint, USB_R_EPnR_CTR_RX);
        } else {
            // Data successfully transmitted (IN transaction complete)
            ev.t = PeripheralEvent::EV_TRANSMIT_COMPLETE;
            ev.addr = EP_IN | endpoint;
            usbd_clearEndpointFlag(endpoint, USB_R_EPnR_CTR_TX);
        }

        return true;
    }

    return false;
}

void STM32USBPeripheral::handleReset() {
    // Reset packet buffer allocation pointer, skipping over initial buffer table:
    m_bufferPtr = USB_BUFFER_TABLE_LEN;

    // Setup buffers for control endpoint:
    constexpr size_t controlRxPtr = USB_BUFFER_TABLE_LEN;
    constexpr size_t controlRxLength = usbd_alignReceiveLength(usbd_alignLength(UB_USBD_MAX_CONTROL_PACKET));
    usbd_setRxDescriptor(USB_BUFFER_TABLE[0], controlRxPtr, usbd_encodeReceiveLength(controlRxLength));

    m_bufferPtr = controlRxPtr + controlRxLength;
    m_out[0].bufferOffset = controlRxPtr;
    m_out[0].bufferLength = UB_USBD_MAX_CONTROL_PACKET;

    constexpr size_t txLength = usbd_alignLength(UB_USBD_MAX_CONTROL_PACKET);
    m_in[0].bufferOffset = m_bufferPtr;
    m_in[0].bufferLength = UB_USBD_MAX_CONTROL_PACKET;

    m_bufferPtr += txLength;

    // Mark EP0 as a control endpoint with EA=0, set NAK response
    USB_R_EPnR[0] = (USB_R_EPnR_TYPE_CONTROL_Val << USB_R_EPnR_TYPE_Pos)
            | (USB_R_EPnR_STAT_NAK_Val << USB_R_EPnR_STAT_RX_Pos)
            | (USB_R_EPnR_STAT_NAK_Val << USB_R_EPnR_STAT_TX_Pos);

    // Initialize zero address and enable USB peripheral:
    USB_R_DADDR = USB_R_DADDR_EF;
}

#endif // defined(UB_USBD_FS_CORE)
