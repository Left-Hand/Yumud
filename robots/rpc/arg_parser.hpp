#pragma once

#include "core/string/string.hpp"
#include "hal/bus/uart/uarthw.hpp"

namespace ymd{

class ArgSplitter{
private:
    String temp_ = "";
    std::vector<StringView> args_;
public:
    ArgSplitter(){;}
    std::optional<const StringViews> update(hal::Uart & input);
    void clear(){temp_ = "";}
};

}