#pragma once

#include "ostream.hpp"

namespace ymd{
class DummyOutputStream final: public OutputStream{
public:
    DummyOutputStream(){;}
    size_t free_capacity() const {return UINT16_MAX;}
    void sendout(const std::span<const char> pbuf){;}
};
    
}