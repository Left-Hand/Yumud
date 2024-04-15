#ifndef __CAN_FILTER_HPP__

#define __CAN_FILTER_HPP__

#include "src/platform.h"

struct CanFilter{
    uint16_t id = 0;
    uint16_t mask = 0;
public:
    CanFilter() = default;
    CanFilter(const uint16_t & _id): id(_id), mask(_id){;}
    CanFilter(const uint16_t & _id, const uint16_t & _mask) : id(_id), mask(_mask){;}
};


#endif