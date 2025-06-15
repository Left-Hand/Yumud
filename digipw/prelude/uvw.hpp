#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"

namespace ymd::digipw{

struct UvwValue{
    q20 u = {};
    q20 v = {};
    q20 w = {};

    q20 operator [](const size_t idx) const {
        return *(&u + idx);
    }

    q20 & operator [](const size_t idx){
        return *(&u + idx);
    }

    friend OutputStream & operator << (OutputStream & os, const UvwValue & self){
        return os << os.brackets<'('>() << 
            self.u << os.splitter() << 
            self.v <<  os.splitter() << 
            self.w << os.brackets<')'>();
    }
};


struct UvwCurrent: public UvwValue{};
struct UvwVoltage: public UvwValue{};

}