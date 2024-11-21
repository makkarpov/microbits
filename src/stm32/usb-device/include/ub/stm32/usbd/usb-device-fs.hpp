#ifndef UB_STM32_USB_DEVICE_STM32_USBD_USB_DEVICE_FS_H
#define UB_STM32_USB_DEVICE_STM32_USBD_USB_DEVICE_FS_H

#include <ub/usbd/static-config.hpp>

#define UB_STM32_USB_DEVICE_FS

#if !defined(UB_STM32_USBD_USB_DEVICE_INCLUDED)
#error "Please include generic <ub/stm32/usb-device.hpp> instead of this file."
#endif

#if (UB_USBD_MAX_OUT_ENDPOINTS > 8) || (UB_USBD_MAX_IN_ENDPOINTS > 8)
#error "USB_FS devices support maximum 8 IN and 8 OUT endpoints"
#endif

namespace ub::stm32::usb {
    class STM32USBPeripheral: public ub::usbd::PeripheralController {
    public:
        explicit constexpr STM32USBPeripheral(): m_bufferPtr(0), m_in {}, m_out {} {}

        void initialize(THROWS) override;
        bool pullEvent(usbd::PeripheralEvent &ev) override;
        void configureDevice(const void *configData, uint8_t speedIndex, THROWS) override;

#if UB_USBD_HAVE_DATA_ENDPOINTS
        void openEndpoint(const ub::usbd::config::EndpointConfig &config, THROWS);
#endif

        void connect() override;
        void disconnect() override;
        void setAddress(uint8_t address, SetAddressPhase phase) override;
        void receivePacket(uint8_t endpoint, void *target) override;
        void transmitPacket(uint8_t endpoint, const void *buffer, size_t length) override;
        void stallEndpoint(uint8_t address, bool stall) override;
        bool stalled(uint8_t address) override;

    private:
        struct EndpointData {
            uint16_t bufferOffset;  //! Byte offset of endpoint's buffer in the USB packet memory
            uint16_t bufferLength;  //! Byte length of endpoint's buffer (max packet length)
            void     *target;       //! Buffer to place the received data
        };

        uint16_t        m_bufferPtr;
        EndpointData    m_in[UB_USBD_MAX_IN_ENDPOINTS];
        EndpointData    m_out[UB_USBD_MAX_OUT_ENDPOINTS];

        void handleReset();
    };
}

#endif // UB_STM32_USB_DEVICE_STM32_USBD_USB_DEVICE_FS_H
