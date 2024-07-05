#pragma once

#include "../bus.hpp"
#include "sys/kernel/stream.hpp"

class Sdi:public OutputStream{

public:

    void write(const char data) override;

    void write(const char * data_ptr, const size_t len) override;

    size_t pending() const override{
        return 0;
    }

    void init();
};

#ifdef HAVE_SDI
extern Sdi sdi;
#endif