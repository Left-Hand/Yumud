#include "can_msg.hpp"
#include "core/stream/ostream.hpp"


namespace ymd{

OutputStream & operator<<(OutputStream & os, const hal::CanMsg & msg){
    const auto guard = os.create_guard();
    
    os << os.brackets<'{'>();

    os << '<'
        << ((msg.is_std()) ? 'S' : 'E')
        << ((msg.is_remote()) ? 'R' : 'D')
        << "> ";

    os << "id=" << std::hex << std::showbase << uint32_t(msg.id_as_u32());

    if(not msg.is_remote()){
        os << " | buf" 
            << '[' << std::dec << msg.size() << "]=" 
            << std::hex << std::span<const uint8_t>{msg.begin(), msg.size()};
    }

    return os << os.brackets<'}'>();
}

}