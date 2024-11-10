#pragma once

#include "../bus.hpp"

namespace yumud{
class Sdi:public OutputStream{

public:

    void write(const char data) override;

    void write(const char * data_ptr, const size_t len) override;

    size_t pending() const override{
        return 0;
    }

    void init();
};

#ifdef ENABLE_SDI
extern Sdi sdi;
#endif
}