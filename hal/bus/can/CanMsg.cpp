#include "CanMsg.hpp"
#include "sys/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.createGuard();

    // os << os.brackets<'{'>() << std::hex;
    os << os.brackets<'{'>();

    // if(msg.isStd()) os << std::bitset<11>(msg.id());
    // else os << std::bitset<29>(msg.id());

    os.setRadix(16);
    os << uint32_t(msg.id());
        
    os << '<'
        << ((msg.isStd()) ? 'S' : 'E')
        << ((msg.isRemote()) ? 'R' : 'D') << std::noshowbase
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex << std::span<const uint8_t>{msg.begin(), msg.size()};

    return os << os.brackets<'}'>();
}

}