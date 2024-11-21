#ifndef UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_STD_H
#define UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_STD_H

#include <ub/usbd/control.hpp>
#include <ub/usbd/static-config.hpp>

namespace ub::usbd {
    class USBDevice;
}

namespace ub::usbd::impl {
    class ByteStreamer: public OutboundControlStreamer {
    public:
        constexpr ByteStreamer(): endpoint(nullptr), data(nullptr), length(0) {}

        void transmitComplete() override;

        void setRegularDescriptor(const uint8_t *data);
        void setConfigDescriptor(const uint8_t *data);

        ControlEndpoint *endpoint;
        const uint8_t   *data;
        size_t          length;
    };

    class StringStreamer: public OutboundControlStreamer {
    public:
        constexpr StringStreamer(): endpoint(nullptr), str(nullptr), length(0), first(false) {}

        void transmitComplete() override;

        ControlEndpoint *endpoint;
        const char      *str;
        size_t          length;
        bool            first;
    };

    class StandardControlHandler: public ControlHandler {
    public:
        constexpr StandardControlHandler():
            m_device(nullptr), m_func(ControlFunc::DEFAULT), m_streamer { .byte {} }
        {}

        void setupControl(ControlRequest &request) override;
        void handleControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS) override;
        void completeControl(const SetupPacket &setup, bool success) override;

    private:
        enum class ControlFunc: uint8_t {
            DEFAULT     = 0,    //! As specified in setup request
            SET_ADDRESS = 1,    //! Address assignment
            SET_STALL   = 2,    //! Set or clear endpoint stall
            GET_STATUS  = 3,    //! Query status
        };

        USBDevice       *m_device;
        ControlFunc     m_func;

        union {
            ByteStreamer    byte;

#if UB_USBD_RUNTIME_SERIAL_NUMBER
            StringStreamer  str;
#endif
        } m_streamer;

        void setupGetDescriptor(ControlRequest &request);
        void setupStringDescriptor(ControlRequest &request, uint8_t index);
        void setupFeature(ControlRequest &request);

        void handleDefaultControl(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS);
        void handleGetStatus(const SetupPacket &setup, uint8_t *buffer, uint32_t &length, THROWS);

        friend class ub::usbd::USBDevice;
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_STD_H
