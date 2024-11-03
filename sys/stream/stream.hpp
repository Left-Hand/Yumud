#pragma once

#include "istream.hpp"
#include "ostream.hpp"

namespace yumud{
class IOStream:public OutputStream, public InputStream{
public:
    IOStream(){;}
    using InputStream::read;
};

}