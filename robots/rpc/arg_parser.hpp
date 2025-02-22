#pragma once

#include "sys/string/string.hpp"
#include "sys/stream/stream.hpp"

namespace ymd{

class ArgSplitter{
private:
    String temp = "";
public:
    ArgSplitter(){;}
    std::optional<const StringViews> update(InputStream & _input);
    void clear(){temp = "";}
};

}