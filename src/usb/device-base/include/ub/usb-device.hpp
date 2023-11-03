#ifndef UB_USB_DEVICE_BASE_USB_DEVICE_H
#define UB_USB_DEVICE_BASE_USB_DEVICE_H

#include <ub/usbd/errors.hpp>
#include <ub/usbd/pcd-interface.hpp>
#include <ub/usbd/impl/static-descriptor.hpp>
#include <ub/usbd/impl/control-endpoint.hpp>
#include <ub/usbd/impl/control-std.hpp>
#include <ub/usbd/function.hpp>

#include <utility>
#include <iterator>

namespace ub::usbd {
    /**
     * Primary USB device class.
     *
     * This class is NOT thread-safe. All calls to methods of this class (as well as indirect calls made by associated
     * function classes) must be synchronized externally. This includes `processEvents` method which is typically
     * triggered by a hardware interrupt.
     */
    class USBDevice {
    public:
        constexpr explicit USBDevice():
            m_pcd(nullptr), m_desc(nullptr), m_func {}, m_funcLogic {}, m_control {}, m_ctrlHandler {},
            m_serialNumber(nullptr), m_funcIdx(0), m_configured(CfgState::RESET)
        {}

        /**
         * Register a runtime function implementation.
         *
         * Functions must be registered in the exact order as they are specified in the descriptor.
         */
        void registerFunction(Function &fn);

        /**
         * Initialize the USB peripheral.
         *
         * Must be called after all functions are registered.
         */
        void initialize(PeripheralController &peripheral, const StaticDescriptor &descriptor, THROWS);

        /**
         * Set device serial number as an ASCII string.
         *
         * Effective only when descriptor is generated with runtime serial number support.
         */
        void setSerialNumber(const char *serial) { m_serialNumber = serial; }

        /**
         * Start the USB stack and connect device to the bus.
         *
         * Must be called after initialization.
         */
        void start();

        /**
         * Stop the USB stack and disconnect device from the bus.
         *
         * All functions are reset to their default state.
         */
        void stop();

        /**
         * Process peripheral events.
         *
         * This method is usually triggered by a hardware interrupt associated with the USB peripheral. External
         * synchronization is required when processing such interrupt, e.g. by deferring the processing to a dedicated
         * RTOS task and using regular RTOS mutexes to synchronize it with other tasks in the system.
         */
        void processEvents();

    private:
        using StdControlHandler = impl::StandardControlHandler;
        using ControlEndpointImpl = impl::ControlEndpointImpl;

        enum class CfgState : uint8_t {
            RESET           = 0,    //! Device was RESET
            DECONFIGURED    = 1,    //! Device was configured, but configuration was reset by host
            CONFIGURED      = 2     //! Device is configured now
        };

        struct FnHostImpl: public FunctionHost {
            constexpr FnHostImpl(): dev(nullptr) {}

            USBDevice *dev;

            [[nodiscard]] LinkSpeed linkSpeed() const override;
            void stallEndpoint(uint8_t endpoint, bool stall) override;
            bool stalled(uint8_t endpoint) override;
            void receivePacket(uint8_t endpoint, void *buffer) override;
            void transmitPacket(uint8_t endpoint, const void *buffer, uint32_t length) override;

        private:
            inline uint32_t functionIdx() { return std::distance(dev->m_funcHosts, this); }
            inline const descriptor::StaticFunction &functionDesc() { return dev->m_desc->functions[functionIdx()]; }
        };

        PeripheralController    *m_pcd;
        const StaticDescriptor  *m_desc;
        Function                *m_func[UB_USBD_MAX_FUNCTIONS];
        FunctionLogic           *m_funcLogic[UB_USBD_MAX_FUNCTIONS];
        FnHostImpl              m_funcHosts[UB_USBD_MAX_FUNCTIONS];
        ControlEndpointImpl     m_control;
        StdControlHandler       m_ctrlHandler;
        const char              *m_serialNumber;

        uint8_t     m_funcIdx;
        CfgState    m_configured;

        void processReset(LinkSpeed speed);
        void processPacketReceived(const PeripheralEvent::RxPacket &ev);
        void processTransmitComplete(uint8_t endpoint);

        void setConfigured(THROWS);
        bool validateEndpoint(uint32_t endpoint);

        ControlHandler *resolveControl(ControlRequest &req, SetupPacket &setup);

        friend class impl::ControlEndpointImpl;
        friend class impl::StandardControlHandler;
    };
}

#endif // UB_USB_DEVICE_BASE_USB_DEVICE_H
