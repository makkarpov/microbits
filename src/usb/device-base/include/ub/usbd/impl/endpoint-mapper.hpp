#ifndef UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H
#define UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H

#include <ub/usbd/static-config.hpp>

#include <cstddef>
#include <cstdint>

namespace ub::usbd::impl {
    using ResourceMapping = ub::usbd::config::ResourceMapping;

    struct LogicalIndex {
        // Use a single 32-bit field to preserve scalar returns
        uint32_t result;    //! Encoded mapping result

        /** @return Whether mapping was successful */
        inline explicit operator bool() const { return (result & PRESENCE_FLAG) != 0; }

        /** @return Mapped function number, zero based */
        [[nodiscard]] inline uint32_t function() const { return (result >> 8) & 0xFF; }

        /** @return Mapped value */
        [[nodiscard]] inline uint8_t value() const { return result & 0xFF; }

        /** @return Encoded logical value */
        static inline LogicalIndex of(uint8_t function, uint8_t value) {
            return LogicalIndex { (uint32_t) (PRESENCE_FLAG | (function << 8) | value) };
        }

        constexpr static uint32_t PRESENCE_FLAG = 0x80000000;
    };

    LogicalIndex toLogicalEndpoint(uint8_t physical, const ResourceMapping &mapping);
    uint8_t toPhysicalEndpoint(uint8_t functionIdx, uint8_t logical, const ResourceMapping &mapping);
    LogicalIndex toLogicalInterface(uint8_t physical, const ResourceMapping &mapping);
}

#endif // UB_USB_DEVICE_BASE_USBD_IMPL_ENDPOINT_MAPPER_H
