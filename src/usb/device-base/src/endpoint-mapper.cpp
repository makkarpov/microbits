#include <ub/usbd/impl/endpoint-mapper.hpp>

using namespace ub::usbd;
using namespace ub::usbd::impl;

constexpr static LogicalIndex LOGICAL_NOT_FOUND { 0xFFFFFFFF };

#ifdef UB_USBD_HAVE_DATA_ENDPOINTS

static LogicalIndex usbd_transformMapping(uint8_t mappedV, uint8_t resultFlag) {
    if (mappedV == EndpointMapping::NOT_MAPPED) {
        return LOGICAL_NOT_FOUND;
    }

    return LogicalIndex::encode(
            EndpointMapping::functionIdx(mappedV),
            EndpointMapping::mappedIdx(mappedV) | resultFlag
    );
}

LogicalIndex impl::toLogicalEndpoint(uint8_t physical, const EndpointMapping &mapping) {
    size_t length;
    const uint8_t *map;

    if ((physical & EP_IN) != 0) {
        map = mapping.in;
        length = UB_USBD_MAX_IN_ENDPOINTS;
    } else {
        map = mapping.out;
        length = UB_USBD_MAX_OUT_ENDPOINTS;
    }

    uint8_t idx = physical & EP_NUM;
    if (idx >= length) {
        return LOGICAL_NOT_FOUND;
    }

    return usbd_transformMapping(map[idx], physical & EP_IN);
}

uint8_t impl::toPhysicalEndpoint(uint8_t logical, const StaticFunction &fn) {
    if (logical >= UB_USBD_MAX_FUNC_ENDPOINTS) {
        return 0xFF;
    }

    return ~fn.endpoints[logical];
}

LogicalIndex impl::toLogicalInterface(uint8_t physical, const EndpointMapping &mapping) {
    if (physical > UB_USBD_MAX_INTERFACES) {
        return LOGICAL_NOT_FOUND;
    }

    return usbd_transformMapping(mapping.intf[physical], 0);
}

#endif // UB_USBD_HAVE_DATA_ENDPOINTS
