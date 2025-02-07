#include "arg_parser.hpp"


using namespace ymd;

std::optional<Strings> ArgSplitter::update(InputStream & is){
    while(is.available()){
        auto chr = is.read();
        if(chr == 0) continue;
        temp += chr;
        if(chr == '\n'){
            temp.alphanum();
            auto args = temp.split(' ');
            if(args.size()){
                return args;
            }
        }
    }
    return std::nullopt;
}