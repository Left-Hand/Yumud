#pragma once

#include "hal/bus/bus.hpp"

namespace ymd{
class Sdi:public OutputStream{

public:

    void write(const char data) override;

    void write(const char * data_ptr, const size_t len) override;

    size_t pending() const override;

    void init();
};

#ifdef ENABLE_SDI
extern Sdi sdi;
#endif
}