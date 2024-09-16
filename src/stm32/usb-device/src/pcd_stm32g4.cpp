#if defined(STM32G4)

#include <ub/stm32/usb-device.hpp>
#include "pcd_stm32g4.hpp"

#include <cstring>
#include <algorithm>

using namespace ub::usbd;
using namespace ub::stm32::usb;

enum {
    EPnR_BASE_MASK      = USB_EP_CTR_RX | USB_EP_CTR_TX,
    EPnR_KEEP_MASK      = USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD,
    EPnR_TOGGLE_MASK    = USB_EPRX_STAT | USB_EPTX_STAT | USB_EP_DTOG_RX | USB_EP_DTOG_TX,

    EPnR_STAT_TX_Pos = 4,
    EPnR_STAT_RX_Pos = 12,
    EPnR_STAT_Msk    = 0x03,

    EPnR_STAT_STALL     = 0x01,
    EPnR_STAT_NAK       = 0x02,
    EPnR_STAT_VALID     = 0x03,
};

static uint16_t usbd_storeReceiveLength(uint8_t endpoint, uint16_t length) {
    if (length <= 62) {
        length = (length + 1) & 0x3E;       // align length up to 2 byte boundary
        USB_BUFFER_TABLE[endpoint].COUNT_RX = length << 9;
    } else {
        length = (length + 31) & 0x1E0;     // align length up to 32 byte boundary

        uint32_t blk = (length >> 5) - 1;   // 0 = 32, 1 = 64, ...
        USB_BUFFER_TABLE[endpoint].COUNT_RX = (blk << 10) | 0x8000;
    }

    return length;
}

static uint16_t usbd_alignTransmitLength(uint16_t length) {
    return (length + 1) & 0x3FE;
}

static void usbd_setEndpointStatus(uint8_t address, uint32_t status) {
    uint16_t epnr_rd = USB_EPnR(address & EP_NUM);
    uint16_t epnr_wr = EPnR_BASE_MASK | (epnr_rd & EPnR_KEEP_MASK);

    uint32_t statPos = ((address & EP_IN) != 0) ? EPnR_STAT_TX_Pos : EPnR_STAT_RX_Pos;
    uint32_t stat = (epnr_rd >> statPos) & EPnR_STAT_Msk;
    epnr_wr |= (stat ^ status) << statPos;

    USB_EPnR(address & EP_NUM) = epnr_wr;
}

static void usbd_clearEndpointFlag(uint8_t address, uint32_t flag) {
    uint16_t epnr = USB_EPnR(address) & EPnR_KEEP_MASK;
    USB_EPnR(address) = (EPnR_BASE_MASK | epnr) & ~flag;
}

/**
 * std::memcpy variation which operates on 16-bit half-words only.
 * Reference manual forbids 32-bit accesses to USB packet memory (data is silently corrupted).
 */
static void usbd_memcpy16(void *dst, const void *src, size_t length) {
    auto dst16 = (uint16_t *) dst;
    auto src16 = (const uint16_t *) src;
    auto end16 = src16 + (length >> 1);

    while (src16 != end16) {
        *dst16 = *src16;
        src16++;
        dst16++;
    }

    if ((length & 1) != 0) {
        *((uint8_t *) dst16) = *((const uint8_t *) src16);
    }
}

void STM32USBPeripheral::initialize(THROWS) {
    // Enable USB clock and remove reset:
    SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN);
    CLEAR_BIT(RCC->APB1RSTR1, RCC_APB1RSTR1_USBRST);
    NVIC_DisableIRQ(USB_HP_IRQn);
    NVIC_DisableIRQ(USB_LP_IRQn);

    // Power-up and reset the transceiver & force CNTR to the known state:
    USB->CNTR = USB_CNTR_FRES;

    // Wait for some time as asked by the reference manual
    volatile uint32_t timeout = 100000;
    while (timeout != 0) timeout--;

    USB->CNTR = 0; // Release USB reset condition
    USB->ISTR = 0; // Clear all pending interrupts
    USB->BTABLE = 0;

    // Clear USB memory for more convenient debugging:
    std::memset(USB_MEMORY, 0, USB_MEMORY_LENGTH);
}

bool STM32USBPeripheral::pullEvent(PeripheralEvent &ev) {
    uint32_t status = USB->ISTR;

    if ((status & USB_ISTR_RESET) != 0) {
        USB->ISTR = USB_CLR_RESET;

        handleReset();

        ev.t = PeripheralEvent::EV_RESET;
        ev.speed = LinkSpeed::FULL;
        return true;
    }

    if ((status & USB_ISTR_CTR) != 0) {
        uint8_t endpoint = status & USB_ISTR_EP_ID;

        if ((status & USB_ISTR_DIR) != 0) {
            // Data received (OUT transaction)
            ev.t = PeripheralEvent::EV_PACKET_RECEIVED;

            ev.packet.addr = endpoint;
            ev.packet.setup = (USB_EPnR(endpoint) & USB_EP_SETUP) != 0;
            ev.packet.size = USB_BUFFER_TABLE[endpoint].COUNT_RX & USB_COUNT0_RX_COUNT0_RX;

            usbd_memcpy16(m_out[endpoint].target, USB_MEMORY + m_out[endpoint].bufferOffset, ev.packet.size);
            usbd_clearEndpointFlag(endpoint, USB_EP_CTR_RX);
        } else {
            // Data successfully transmitted (IN transaction complete)
            ev.t = PeripheralEvent::EV_TRANSMIT_COMPLETE;
            ev.addr = EP_IN | endpoint;
            usbd_clearEndpointFlag(endpoint, USB_EP_CTR_TX);
        }

        return true;
    }

    return false;
}

#if UB_USBD_HAVE_DATA_ENDPOINTS
enum { EP_TYPE_SHIFT = 8 };
static const uint8_t usbd_endpointTypes[4] = {
        USB_EP_CONTROL >> EP_TYPE_SHIFT,    USB_EP_ISOCHRONOUS >> EP_TYPE_SHIFT,
        USB_EP_BULK >> EP_TYPE_SHIFT,       USB_EP_INTERRUPT >> EP_TYPE_SHIFT
};

void STM32USBPeripheral::openEndpoint(const EndpointConfig &config, THROWS) {
    uint8_t epNumber = config.address & EP_NUM;

    uint16_t epnr = EPnR_BASE_MASK | (USB_EPnR(epNumber) & EPnR_KEEP_MASK);
    epnr = (epnr & ~USB_EP_TYPE_MASK) | (usbd_endpointTypes[(int) config.type] << EP_TYPE_SHIFT);
    epnr = (epnr & ~USB_EPADDR_FIELD) | (config.address & 0xF);
    USB_EPnR(epNumber) = epnr;

    if ((config.address & EP_IN) != 0) {
        USB_BUFFER_TABLE[epNumber].ADDR_TX = m_bufferPtr;
        m_in[epNumber].bufferOffset = m_bufferPtr;
        m_in[epNumber].bufferLength = config.maxPacket;
        m_bufferPtr += usbd_alignTransmitLength(config.maxPacket);
    } else {
        USB_BUFFER_TABLE[epNumber].ADDR_RX = m_bufferPtr;
        m_out[epNumber].bufferOffset = m_bufferPtr;
        m_bufferPtr += usbd_storeReceiveLength(config.address, config.maxPacket);
    }

    usbd_setEndpointStatus(config.address, EPnR_STAT_NAK);
}
#endif

void STM32USBPeripheral::connect() {
    SET_BIT(USB->CNTR, USB_CNTR_RESETM | USB_CNTR_CTRM);
    SET_BIT(USB->BCDR, USB_BCDR_DPPU);
}

void STM32USBPeripheral::disconnect() {
    CLEAR_BIT(USB->BCDR, USB_BCDR_DPPU);

    USB->CNTR = 0;
    USB->DADDR = 0;
    USB->ISTR = 0;
}

void STM32USBPeripheral::setAddress(uint8_t address, SetAddressPhase phase) {
    if (phase != SetAddressPhase::STATUS_ACKNOWLEDGED) {
        return;
    }

    USB->DADDR = address | USB_DADDR_EF;
}

void STM32USBPeripheral::receivePacket(uint8_t endpoint, void *target) {
    m_out[endpoint].target = target;
    usbd_setEndpointStatus(endpoint, EPnR_STAT_VALID);
}

void STM32USBPeripheral::transmitPacket(uint8_t endpoint, const void *buffer, size_t length) {
    endpoint &= EP_NUM;

    length = std::min(length, (size_t) m_in[endpoint].bufferLength);
    usbd_memcpy16(USB_MEMORY + m_in[endpoint].bufferOffset, buffer, length);
    USB_BUFFER_TABLE[endpoint].COUNT_TX = length;

    usbd_setEndpointStatus(EP_IN | endpoint, EPnR_STAT_VALID);
}

void STM32USBPeripheral::stallEndpoint(uint8_t address, bool stall) {
    usbd_setEndpointStatus(address, stall ? EPnR_STAT_STALL : EPnR_STAT_NAK);
}

bool STM32USBPeripheral::stalled(uint8_t address) {
    uint16_t epnr = USB_EPnR(address & EP_NUM);
    uint32_t statPos = ((address & EP_IN) != 0) ? EPnR_STAT_TX_Pos : EPnR_STAT_RX_Pos;
    return ((epnr >> statPos) & EPnR_STAT_Msk) == EPnR_STAT_STALL;
}

void STM32USBPeripheral::handleReset() {
    // Reset packet buffer allocation pointer, skipping over initial buffer table:
    m_bufferPtr = USB_BUFFER_TABLE_LEN;

    for (size_t i = 0; i < USB_N_ENDPOINTS; i++) {
        // Clear all EPnR registers by echoing all active toggled bits (thus toggling them back to zero):
        USB_EPnR(i) = USB_EPnR(i) & EPnR_TOGGLE_MASK;
    }

    // Setup buffers for control endpoint:
    uint16_t ptrRx = m_bufferPtr;
    USB_BUFFER_TABLE[0].ADDR_RX = ptrRx;
    m_bufferPtr += usbd_storeReceiveLength(0, UB_USBD_MAX_CONTROL_PACKET);
    m_out[0].bufferOffset = ptrRx;
    m_out[0].bufferLength = UB_USBD_MAX_CONTROL_PACKET;

    uint16_t ptrTx = m_bufferPtr;
    m_bufferPtr += usbd_alignTransmitLength(UB_USBD_MAX_CONTROL_PACKET);
    USB_BUFFER_TABLE[0].ADDR_TX = ptrTx;
    USB_BUFFER_TABLE[0].COUNT_TX = 0;
    m_in[0].bufferOffset = ptrTx;
    m_in[0].bufferLength = UB_USBD_MAX_CONTROL_PACKET;

    // Mark EP0 as a control endpoint with EA=0, set NAK response
    USB->EP0R = USB_EP_CONTROL | (EPnR_STAT_NAK << EPnR_STAT_RX_Pos) | (EPnR_STAT_NAK << EPnR_STAT_TX_Pos);

    // Initialize zero address and enable USB peripheral:
    USB->DADDR = USB_DADDR_EF;
}

#endif // defined(STM32G4)
