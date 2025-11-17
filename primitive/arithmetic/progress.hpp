#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"

namespace ymd{
struct Progress{
    size_t current;
    size_t total;

    friend OutputStream & operator <<(OutputStream & os, const Progress & self){
        return os << os.brackets<'['>() << 
            self.current << os.splitter() << self.total 
            << os.brackets<']'>();
    }
};

}