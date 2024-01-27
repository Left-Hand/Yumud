#ifndef __UINT24_T_H__

#define __UINT24_T_H__

#include "stdint.h"
#include "defines/comm_inc.h"

#pragma pack(push, 1)
struct uint24_t{
    uint32_t data : 24;

#ifdef __cplusplus

    __fast_inline uint24_t() {
        data = 0;
    }

    __fast_inline explicit uint24_t(const uint24_t & other) {
        data = other.data;
    }

    __fast_inline explicit uint24_t(const uint32_t & other) {
        data = other & 0xFFFFFF;
    }

    __fast_inline explicit uint24_t(const int & other) {
        data = other & 0xFFFFFF;
    }

    __fast_inline explicit operator uint32_t() const {
        return data;
    }

#endif
};

typedef struct uint24_t uint24_t;

#pragma pack(pop)

#endif