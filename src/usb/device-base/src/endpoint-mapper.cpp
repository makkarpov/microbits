#include <ub/usbd/impl/endpoint-mapper.hpp>
#include <ub/usbd/model.hpp>

using namespace ub::usbd;
using namespace ub::usbd::impl;

constexpr static LogicalIndex LOGICAL_NOT_MAPPED { 0 };

static LogicalIndex usbd_transformMapping(uint8_t mappedV, uint8_t resultExtra) {
    uint8_t functionIdx = (mappedV >> 4) & 0xF;
    if (functionIdx == 0) {
        return LOGICAL_NOT_MAPPED;
    }

    return LogicalIndex::of(functionIdx - 1, (mappedV & 0xF) | resultExtra);
}

LogicalIndex impl::toLogicalEndpoint(uint8_t physical, const ResourceMapping &mapping) {
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
        return LOGICAL_NOT_MAPPED;
    }

    return usbd_transformMapping(map[idx], physical & EP_IN);
}

uint8_t impl::toPhysicalEndpoint(uint8_t functionIdx, uint8_t logical, const ResourceMapping &mapping) {
    if (logical >= UB_USBD_MAX_FUNC_ENDPOINTS) {
        return 0xFF;
    }

    return mapping.funcEndpoints[functionIdx][logical];
}

LogicalIndex impl::toLogicalInterface(uint8_t physical, const ResourceMapping &mapping) {
    if (physical > UB_USBD_MAX_INTERFACES) {
        return LOGICAL_NOT_MAPPED;
    }

    return usbd_transformMapping(mapping.interfaces[physical], 0);
}
