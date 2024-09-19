#pragma once

#include "istream.hpp"
#include "ostream.hpp"

class IOStream:public OutputStream, public InputStream{
public:
    using InputStream::read;
};