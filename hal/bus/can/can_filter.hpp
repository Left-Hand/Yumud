#ifndef __CAN_FILTER_HPP__

#define __CAN_FILTER_HPP__

#include "sys/platform.h"
#include <initializer_list>

struct CanFilter{
protected:
    union{
        uint16_t id16[2];
        uint32_t id32;
    };

    union{
        uint16_t mask16[2] = {0};
        uint32_t mask32;
    };

public:
    CanFilter() = default;
    CanFilter(const uint16_t _id): id16{_id, 0}, mask16{_id, 0}{;}
    CanFilter(const uint16_t _id, const uint16_t _mask) : id16{_id, 0}, mask16{_mask, 0}{;}
    CanFilter(const std::initializer_list<uint16_t> & list);

    void init();
    static void init(const CanFilter & filter);
};


#endif