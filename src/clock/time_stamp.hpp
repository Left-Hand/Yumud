#ifndef __TIME_STAMP_HPP__

#define __TIME_STAMP_HPP__

#include "clock.h"


class TimeStamp{
protected:
    const uint32_t begin;
public:
    TimeStamp():begin(micros()){;}
    operator uint32_t() const {
        uint32_t time_current = micros();
        return (time_current + ((time_current ^ begin) & 0x8000? begin : - begin));
    }
};


#endif