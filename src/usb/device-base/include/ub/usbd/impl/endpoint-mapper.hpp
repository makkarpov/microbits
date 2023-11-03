#ifndef UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H
#define UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H

#include <ub/usbd/impl/static-descriptor.hpp>

#include <cstddef>
#include <cstdint>

namespace ub::usbd::impl {
    using EndpointMapping = ub::usbd::descriptor::EndpointMapping;
    using StaticFunction = ub::usbd::descriptor::StaticFunction;

    struct LogicalIndex {
        // Use a single 32-bit field to preserve scalar returns
        uint32_t result;    //! Encoded mapping result

        /** @return Whether mapping was successful */
        inline explicit operator bool() const { return (result >> 31) == 0; }

        /** @return Mapped function number, zero based */
        [[nodiscard]] inline uint32_t function() const { return (result >> 8) & 0xFF; }

        /** @return Mapped value number */
        [[nodiscard]] inline uint8_t value() const { return result & 0xFF; }

        /** @return Encoded logical index value */
        static inline LogicalIndex encode(uint8_t function, uint8_t value) {
            return { (uint32_t) ((function << 8) | value) };
        }
    };

    LogicalIndex toLogicalEndpoint(uint8_t physical, const EndpointMapping &mapping);
    uint8_t toPhysicalEndpoint(uint8_t logical, const StaticFunction &fn);

    LogicalIndex toLogicalInterface(uint8_t physical, const EndpointMapping &mapping);
}

#endif // UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H
