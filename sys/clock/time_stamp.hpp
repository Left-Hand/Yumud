#pragma once

#include "clock.h"


class TimeStamp{
protected:
    const uint32_t begin;
public:
    TimeStamp():begin(micros()){;}
    TimeStamp(const TimeStamp& other) = default;
    operator uint32_t() const {
        uint32_t time_current = micros();
        return (time_current + ((time_current ^ begin) & 0x8000? begin : - begin));
    }
};
