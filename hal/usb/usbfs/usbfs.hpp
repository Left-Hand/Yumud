#pragma once

#include "core/stream/ostream.hpp"

#include "hal/bus/bus_base.hpp"

#include <functional>

namespace ymd{

class UsbFS:public OutputStream{
public:

protected:

public:
    void init();
    size_t pending() const {return 0;}
    void sendout(std::span<const char>){;}
};

#if defined(CH32V30X) && defined(ENABLE_USBFS)
extern UsbFS usbfs;
#endif

}