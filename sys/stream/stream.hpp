#pragma once

#include "istream.hpp"
#include "ostream.hpp"

namespace ymd{
class IOStream:public OutputStream, public InputStream{
public:
    IOStream(){;}
    using InputStream::read;
};

}