#pragma once

#include "clock.hpp"

namespace ymd{

class TimeStamp{
protected:
    const uint32_t begin;
public:
    TimeStamp():begin(micros()){;}
    TimeStamp(const TimeStamp& other) = default;
    uint32_t duration() const {
        uint32_t time_current = micros();
        return (time_current + ((time_current ^ begin) & 0x8000? begin : - begin));
    }
};

}