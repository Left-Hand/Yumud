#pragma once

#include "../bus.hpp"
#include "sys/kernel/stream.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"

#include <functional>

class UsbFS:public OutputStream{
public:

protected:

public:
    void init();
    void write(const char data) override;
    void write(const char * data_ptr, const size_t len) override;
    size_t pending() const {return 0;}
};

#ifdef HAVE_USBFS
extern UsbFS usbfs;
#endif