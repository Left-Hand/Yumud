#include "can_msg.hpp"
#include "sys/kernel/stream.hpp"


OutputStream & operator<<(OutputStream & os, const CanRxMsg & msg){
    return os << '{' << 
        std::dec << msg.StdId
        << ((msg.RTR == CAN_RTR_Remote) ? 'R' : 'D') << ',' 
        << std::hex << std::vector<uint8_t>(msg.Data, msg.Data + msg.DLC)
     << '}';
}