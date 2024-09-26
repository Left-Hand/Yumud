#include "can_msg.hpp"
#include "sys/stream/ostream.hpp"
OutputStream & operator<<(OutputStream & os, const CanMsg & msg){
    os << "{0x" << 
        std::hex << msg.StdId << '<'
        << ((msg.isStd()) ? "Std" : "Ext")
        << ((msg.RTR == CAN_RTR_Remote) ? "Rmt" : "Dat")
        << ">,";
    
    for(size_t i = 0; i < msg.size(); i++){
        os << msg[i];
        if(i == msg.size() - 1) break;
        os << ',';
    }
        
    return os << '}';
}