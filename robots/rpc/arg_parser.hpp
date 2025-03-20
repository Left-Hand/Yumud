#pragma once

#include "core/string/string.hpp"
#include "hal/bus/uart/uarthw.hpp"

namespace ymd{

class ArgSplitter{
private:
    String temp = "";
public:
    ArgSplitter(){;}
    std::optional<const StringViews> update(hal::UartHw & _input);
    void clear(){temp = "";}
};

}