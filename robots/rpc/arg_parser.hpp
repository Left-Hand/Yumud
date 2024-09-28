#pragma once

#include "sys/string/string.hpp"
#include "sys/stream/stream.hpp"

class ArgParser{
private:
    String temp = "";
public:
    ArgParser(){;}
    Strings update(InputStream & _input);
    void clear(){temp = "";}
};
