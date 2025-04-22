#include "arg_parser.hpp"
// #include "core/debug/debug.hpp"

using namespace ymd;

std::optional<const StringViews> ArgSplitter::update(hal::Uart & input){
    // if(input.available()) DEBUG_PRINTLN(input.available());
    while(input.available()){
        char chr;
        input.read1(chr);
        if(chr == 0) continue;
        temp_.concat(chr);
        if(chr == '\n'){
            temp_.alphanum();
            args_ = temp_.split(' ');
            if(args_.size()){
                return StringViews{args_.data(), args_.size()};
            }
        }
    }
    return std::nullopt;
}