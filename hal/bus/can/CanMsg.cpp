#include "CanMsg.hpp"
#include "sys/stream/ostream.hpp"


using namespace ymd;

OutputStream & operator<<(OutputStream & os, const CanMsg & msg){
    os << "{0x" << 
        std::hex << msg.id() << '<'
        << ((msg.isStd()) ? "Std" : "Ext")
        << ((msg.isRemote()) ? "Rmt" : "Dat")
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex;

    for(size_t i = 0; i < msg.size(); i++){
        os << msg[i];
        if(i == msg.size() - 1) break;
        os << ',';
    }
    
    os << std::dec;
        
    return os << '}';
}