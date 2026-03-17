#pragma once

#include "clock.hpp"

namespace ymd::clock{

class TimeStamp{
protected:
    const uint32_t begin;
public:
    TimeStamp():begin(clock::micros().count()){;}
    TimeStamp(const TimeStamp & other) = default;
    Milliseconds duration() const {
        const uint32_t time_current = clock::micros().count();

        return Milliseconds(time_current + ((time_current ^ begin) & 0x8000? begin : - begin));
    }
};

}