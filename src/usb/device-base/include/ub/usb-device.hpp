#ifndef UB_USB_DEVICE_BASE_USB_DEVICE_H
#define UB_USB_DEVICE_BASE_USB_DEVICE_H

#include <ub/usbd/errors.hpp>
#include <ub/usbd/function.hpp>
#include <ub/usbd/pcd-interface.hpp>
#include <ub/usbd/static-config.hpp>
#include <ub/usbd/impl/control-endpoint.hpp>
#include <ub/usbd/impl/control-std.hpp>

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
                m_pcd(nullptr), m_cfg(nullptr), m_funcs {}, m_control {}, m_ctrlHandler {},
#if UB_USBD_RUNTIME_SERIAL_NUMBER
                m_serialNumber(nullptr),
#endif
#if UB_USBD_HAVE_RESOURCE_MAPPING
                m_activeMapping(nullptr),
#endif
                m_nextFunctionIdx(0), m_configured(CfgState::RESET)
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
        void initialize(PeripheralController &peripheral, const config::StaticConfiguration &config, THROWS);

#if UB_USBD_RUNTIME_SERIAL_NUMBER
        /**
         * Set device serial number as an ASCII string.
         *
         * Effective only when descriptor is generated with runtime serial number support.
         */
        void setSerialNumber(const char *serial) { m_serialNumber = serial; }
#endif

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
         * Process peripheral events. Returns a bitmask of occurred device-global events, and updates individual
         * functions that might return their own events as well (via their event processing functions).
         *
         * This method is usually triggered by a hardware interrupt associated with the USB peripheral. External
         * synchronization is required when processing such interrupt, e.g. by deferring the processing to a dedicated
         * RTOS task and using regular RTOS mutexes to synchronize it with other tasks in the system.
         *
         * @return Bitmask of occurred events (see `EV_*` constants).
         */
        uint32_t processEvents();

        /** USB reset has been received */
        constexpr static uint32_t EV_RESET                      = 0x00000001;

    private:
        using StdControlHandler = impl::StandardControlHandler;
        using ControlEndpointImpl = impl::ControlEndpointImpl;
        using StaticConfig = config::StaticConfiguration;
        using ResourceMapping = config::ResourceMapping;

        enum class CfgState : uint8_t {
            RESET           = 0,    //! Device was reset (in USB sense) and never configured since
            DECONFIGURED    = 1,    //! Device was configured, but configuration was reset by host
            CONFIGURED      = 2     //! Device is configured now
        };

        struct FnHostImpl: public FunctionHost {
            constexpr FnHostImpl(): dev(nullptr), func(nullptr), logic(nullptr) {}

            USBDevice     *dev;
            Function      *func;
            FunctionLogic *logic;

            [[nodiscard]] LinkSpeed linkSpeed() const override;
            void stallEndpoint(uint8_t endpoint, bool stall) override;
            bool stalled(uint8_t endpoint) override;
            void receivePacket(uint8_t endpoint, void *buffer) override;
            void transmitPacket(uint8_t endpoint, const void *buffer, uint32_t length) override;

        private:
            [[nodiscard]] inline uint32_t functionIdx() const {
                return std::distance((const FnHostImpl *) dev->m_funcs, this);
            }

            [[nodiscard]] uint8_t toPhysicalEndpoint(uint8_t endpoint) const;
        };

        PeripheralController    *m_pcd;
        const StaticConfig      *m_cfg;
        FnHostImpl              m_funcs[UB_USBD_MAX_FUNCTIONS];
        ControlEndpointImpl     m_control;
        StdControlHandler       m_ctrlHandler;

#if UB_USBD_RUNTIME_SERIAL_NUMBER
        const char              *m_serialNumber;
#endif

#if UB_USBD_HAVE_RESOURCE_MAPPING
        const ResourceMapping   *m_activeMapping;
#endif

        uint8_t     m_nextFunctionIdx;
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
