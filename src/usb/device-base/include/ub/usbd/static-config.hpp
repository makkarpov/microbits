#ifndef UB_USB_DEVICE_BASE_USBD_STATIC_CONFIG_H
#define UB_USB_DEVICE_BASE_USBD_STATIC_CONFIG_H

#include <cstdint>
#include <cstddef>

#include <ub/usbd/config.hpp>
#include <ub/usbd/model.hpp>

/** Version number to enforce code regeneration when this file changes */
#define UB_USBD_STATIC_CONFIG_VERSION                           202311110

/**
 * Statically compiled device configuration that provides everything which is infeasible or impossible to compute at
 * runtime. This includes:
 *
 *  1. Endpoint allocation (various target devices place various funny constraints on this)
 *  2. Endpoint and interface mappings for functions, so that functions could deal with consistent logical resource
 *     indices regardless of their actual physical endpoint allocation.
 *  3. All required descriptor arrays (several arrays if device is high-speed capable)
 *  4. Hardware configuration, such as FIFO allocation.
 *
 * All data types and structures listed in this file are considered internal API. The only portable way to obtain
 * their instances is to use the USB descriptor compiler tool.
 */
namespace ub::usbd::config {
#if UB_USBD_ENABLE_HIGH_SPEED
    /** Number of supported interface speeds */
    constexpr static uint32_t N_SPEEDS = 2;
#else
    /** Number of supported interface speeds */
    constexpr static uint32_t N_SPEEDS = 1;
#endif

    /** String descriptor, along with associated metadata */
    struct StringDescriptor {
        /** String index */
        uint8_t         index;

        /**
         * Descriptor data, together with string descriptor header.
         *
         * Length is stored in `bLength` descriptor field (first byte).
         */
        const uint8_t   *data;
    };

    /** Descriptor data provided to USB host */
    struct DescriptorData {
        using string_t = const StringDescriptor *;

        /**
         * Device descriptor data for both full and high speed
         *
         * Length is stored in `bLength` descriptor field (first byte).
         */
        const uint8_t   *deviceDescriptor;

        /**
         * Configuration descriptor data for each speed
         *
         * Length is stored in `wTotalLength` descriptor field (second to fourth bytes).
         */
        const uint8_t   *configDescriptor[N_SPEEDS];

        /** String descriptor data */
        string_t        strings;

        /** Number of entries in `strings` array */
        uint8_t         stringCount;

#if UB_USBD_RUNTIME_SERIAL_NUMBER
        /** Index of serial number string, which is not present in static data and should be generated at runtime. */
        uint8_t         serialNumberIndex;
#endif
    };

    /** Bidirectional mapping between physical and logical USB resources (endpoints and interfaces) */
    struct ResourceMapping {
        // Note - 0x00 must be a well-defined invalid value for all physical-to-logical mappings, as C compiler pads all
        // arrays with zeros (if corresponding configuration parameter is larger than minimum) and malicious host could
        // try sending us invalid numbers

#if UB_USBD_HAVE_DATA_ENDPOINTS
        /**
         * Physical-to-logical mapping for IN endpoints.
         *
         * Stored as `((func_idx + 1) << 4) | mapped_value`, where `mapped_value` is an endpoint number.
         * This also makes `0x00` an invalid value.
         */
        uint8_t         in[UB_USBD_MAX_IN_ENDPOINTS];

        /** Physical-to-logical mapping for OUT endpoints. Stored in the same format as `in` array. */
        uint8_t         out[UB_USBD_MAX_OUT_ENDPOINTS];

        /** Logical-to-physical endpoint mapping for a functions, indexed as `[func_idx][logical_addr]`. */
        uint8_t         funcEndpoints[UB_USBD_MAX_FUNCTIONS][UB_USBD_MAX_FUNC_ENDPOINTS];
#endif

#if UB_USBD_MAX_FUNCTIONS > 1
        /**
         * Mapping for interface numbers to functions and logical interfaces
         *
         * Stored in same way as `in` and `out` arrays, with `mapped_value` being an interface number.
         */
        uint8_t         interfaces[UB_USBD_MAX_INTERFACES];

        /** Base interface offsets for functions. */
        uint8_t         funcInterfaceBase[UB_USBD_MAX_FUNCTIONS];
#endif
    };

    struct FunctionConfig {
#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
        /**
         * Type identifier of a function for which configuration is generated.
         *
         * Used to check consistency between function type specified in descriptor compiler and function implementation
         * provided at runtime.
         *
         * Derived from fully-qualified function identifier string by taking SHA-256 hash and interpreting first
         * four bytes of it as a big-endian integer.
         */
        uint32_t        functionTypeId;
#endif

        /** Function-specific configuration data required to complete function configuration. */
        const void      *configData;
    };

    /** Endpoint configuration structure for 'generic' type targets */
    struct EndpointConfig {
        constexpr static uint8_t FLAG_DOUBLE_BUFFERED   = 0x01;

        /** Endpoint address, with highest bit (`0x80`) indicating endpoint direction. */
        uint8_t         address;

        /** Endpoint type to initialize */
        EndpointType    type;

        /** Endpoint's maximum packet size in bytes */
        uint32_t        maxPacket;

        /** Endpoint flags */
        uint8_t         flags;
    };

    /**
     * `targetData` structure for 'generic' type targets (when `targetId == 0x3A2E8954`).
     */
    struct GenericTargetData {
        using ep_config = const EndpointConfig *;

        /** Configuration has been compiled with high-speed support */
        constexpr static uint8_t FLAG_HIGH_SPEED    = 0x01;

        /** Compilation flags */
        uint8_t         flags;

        /** Number of endpoints present in `endpoints` array */
        uint8_t         endpointCount[N_SPEEDS];

        /** Device endpoints */
        ep_config       endpoints[N_SPEEDS];
    };

    struct StaticConfiguration {
        using fn_config = const FunctionConfig *;

#if UB_USBD_ENABLE_TYPE_IDENTIFIERS
        /**
         * Type identifier of target hardware for which descriptor was compiled
         *
         * Used to check consistence between target selected in descriptor compiler and target implementation provided
         * at runtime.
         *
         * Derived from fully-qualified target hardware identifier string by taking SHA-256 hash and interpreting first
         * four bytes of it as a big-endian integer.
         */
        uint32_t        targetId;
#endif

        /**
         * Target-specific data required to complete USB hardware configuration.
         *
         * This could be a simple list of endpoints, or it could have more data such as FIFO buffer allocation plan.
         */
        const void      *targetData;

        /** Structure with various descriptors required by USB specification */
        DescriptorData  descriptors;

#if UB_USBD_HAVE_RESOURCE_MAPPING
        /** Physical-logical resource mapping for each link speed */
        ResourceMapping mapping[N_SPEEDS];
#endif

        /** Number of valid entries in `functions` array. */
        uint8_t         functionCount;

        /** Configuration for the functions */
        FunctionConfig  functions[UB_USBD_MAX_FUNCTIONS];
    };
}

#endif // UB_USB_DEVICE_BASE_USBD_STATIC_CONFIG_H
