#include "uint256.hpp"

#include <cstring>

using namespace ub::crypto::impl;

uint256_t &uint256_t::operator=(uint32_t v) {
    memset(u32, 0, sizeof(u32));
    u32[0] = v;
    return *this;
}

void uint256_t::select(bool condition, const uint256_t &v_false, const uint256_t &v_true) {
    uint32_t mask = -(condition & 1);   // 0 -> 0x00, 1 -> 0xFF

    for (size_t i = 0; i < N_U32; i++) {
        uint32_t diff = v_false.u32[i] ^ v_true.u32[i];
        u32[i] = v_false.u32[i] ^ (diff & mask);
    }
}

void uint256_t::destroy() {
    secureZero(u8, sizeof(u8));
}

