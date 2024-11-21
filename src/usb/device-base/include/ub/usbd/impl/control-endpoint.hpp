#ifndef UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_ENDPOINT_H
#define UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_ENDPOINT_H

#include <ub/usbd/serial-config.hpp>
#include <ub/usbd/pcd-interface.hpp>
#include <ub/usbd/control.hpp>

namespace ub::usbd {
    class USBDevice;
}

namespace ub::usbd::impl {
    /** Converts negotiated link speed into an index for various configuration/descriptor arrays */
#if UB_USBD_ENABLE_HIGH_SPEED
    inline uint8_t linkSpeedIndex(LinkSpeed speed) {
        return speed == LinkSpeed::HIGH ? 1 : 0;
    }
#else
    inline uint8_t linkSpeedIndex(LinkSpeed speed) {
        return 0;
    }
#endif

    /**
     * Control request handler and state machine.
     *
     * This class acts both as public interface for application-level control handlers and as actual implementation.
     * This is done to reduce overhead for virtual method table where only one implementation exists
     */
    class ControlEndpointImpl: public ControlEndpoint {
    public:
        // Public interface for control handlers:

        [[nodiscard]] const SetupPacket &setupPacket() const override { return m_setup; }
        [[nodiscard]] uint8_t *packetBuffer() override { return m_buffer; }
        [[nodiscard]] size_t packetLength() const override { return m_packetLength; }
        [[nodiscard]] LinkSpeed linkSpeed() const override { return m_speed; }
        void receivePacket() override;
        void transmitPacket(const void *buffer, size_t length) override;
        void abortRequest() override;

    private:
        // Interface exported to USBDevice class

        /** Initialize control processor to it's default state */
        constexpr explicit ControlEndpointImpl():
                m_pcd(nullptr), m_handler(nullptr), m_streamer(nullptr), m_device(nullptr), m_setup {},
                m_packetLength(0), m_dataLength(0), m_buffer {}, m_state(SetupState::IDLE), m_speed(LinkSpeed::NONE)
        {}

        /** Reset state machine */
        void reset(LinkSpeed speed);

        /** Handle received packet */
        void packetReceived(uint32_t length, bool setup);

        /** Handle packet transmission */
        void transmitComplete();

    private:
        // Really private data, even though C++ thinks it's the same 'private' as previous

        enum class SetupState: uint8_t {
            IDLE            = 0,    //! Idle
            TX_MORE_DATA    = 1,    //! Transmitting intermediate chunk of data
            TX_DATA         = 2,    //! Transmitting last chunk of data
            RX_DATA         = 3,    //! Expecting to receive more data
            TX_STATUS       = 4,    //! Transmitting status packet
            RX_STATUS       = 5,    //! Expecting to receive status packet
            WAITING         = 6     //! Streamer has been notified, but didn't perform any actions yet
        };

        PeripheralController *m_pcd;
        ControlHandler       *m_handler;
        ControlStreamer      *m_streamer;
        USBDevice            *m_device;

        SetupPacket          m_setup;
        uint16_t             m_packetLength;    // Length of last received packet
        uint16_t             m_dataLength;      // Total length of current data phase

        uint8_t              m_buffer[UB_USBD_MAX_CONTROL_PACKET];
        SetupState           m_state;
        LinkSpeed            m_speed;

        void setupReceived(uint32_t length, THROWS);
        void packetReceived(uint32_t length, THROWS);
        bool streamReceiveNext(uint32_t length);
        void completeRequest();

        friend class ub::usbd::USBDevice;
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_IMPL_CONTROL_ENDPOINT_H
