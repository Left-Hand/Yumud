#pragma once

#include <cstddef>

namespace ymd::hal{
class Sdi{

public:

    void write(const char data);

    void write(const char * data_ptr, const size_t len);

    size_t pending() const;

    void init();
};

#ifdef ENABLE_SDI
extern Sdi sdi;
#endif
}