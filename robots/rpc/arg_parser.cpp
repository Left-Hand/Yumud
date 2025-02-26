#include "arg_parser.hpp"


using namespace ymd;

std::optional<const StringViews> ArgSplitter::update(hal::UartHw & is){
    while(is.available()){
        char chr;
        is.read1(chr);
        if(chr == 0) continue;
        temp += chr;
        if(chr == '\n'){
            temp.alphanum();
            auto args = temp.split(' ');
            if(args.size()){
                return StringViews{args.data(), args.size()};
            }
        }
    }
    return std::nullopt;
}