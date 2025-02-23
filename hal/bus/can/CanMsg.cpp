#include "CanMsg.hpp"
#include "sys/stream/ostream.hpp"


namespace ymd{

__inline OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.createGuard();

    os << "{" << std::showbase << 
        std::hex << std::bitset<11>(msg.id()) << '<'
        << ((msg.isStd()) ? 'S' : 'E')
        << ((msg.isRemote()) ? 'R' : 'D') << std::noshowbase
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex << std::span<const uint8_t>{msg.begin(), msg.size()};

    return os << '}';
}

}