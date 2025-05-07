#pragma once

#include "ostream.hpp"

namespace ymd{
class DummyOutputStream final: public OutputStream{
public:
    DummyOutputStream(){;}
    size_t pending() const {return 0;}
    void sendout(const std::span<const char> pbuf){;}
};
    
}